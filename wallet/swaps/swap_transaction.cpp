//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "swap_transaction.h"

#include "bitcoin/bitcoin.hpp"

#include "lock_tx_builder.h"
#include "shared_tx_builder.h"
#include "../bitcoin/bitcoin_side.h"

using namespace ECC;

namespace grimm::wallet
{


    AtomicSwapTransaction::WrapperSecondSide::WrapperSecondSide(INegotiatorGateway& gateway, const TxID& txID)
        : m_gateway(gateway)
        , m_txID(txID)
    {
    }

    SecondSide::Ptr AtomicSwapTransaction::WrapperSecondSide::operator -> ()
    {
        if (!m_secondSide)
        {
            m_secondSide = m_gateway.GetSecondSide(m_txID);

            if (!m_secondSide)
            {
                throw UninitilizedSecondSide();
            }
        }

        return m_secondSide;
    }

    AtomicSwapTransaction::AtomicSwapTransaction(INegotiatorGateway& gateway
                                               , IWalletDB::Ptr walletDB
                                               , IPrivateKeyKeeper::Ptr keyKeeper
                                               , const TxID& txID)
        : BaseTransaction(gateway, walletDB, keyKeeper, txID)
        , m_secondSide(gateway, txID)
    {
    }

    void AtomicSwapTransaction::Cancel()
    {
        State state = GetState(kDefaultSubTxID);

        switch (state)
        {
        case State::HandlingContractTX:
            if (!IsGrimmSide())
            {
                break;
            }
        case State::Initial:
        case State::Invitation:
        case State::BuildingGrimmLockTX:
        case State::BuildingGrimmRedeemTX:
        case State::BuildingGrimmRefundTX:
        {
            SetNextState(State::Cancelled);
            return;
        }
        default:
            break;
        }

        LOG_INFO() << GetTxID() << " You cannot cancel transaction in state: " << static_cast<int>(state);
    }

    bool AtomicSwapTransaction::Rollback(Height height)
    {
        Height proofHeight = 0;

        if (IsGrimmSide())
        {
            bool isRolledback = false;
            if (GetParameter(TxParameterID::KernelProofHeight, proofHeight, SubTxIndex::GRIMM_REFUND_TX)
                && proofHeight > height)
            {
                SetParameter(TxParameterID::KernelProofHeight, Height(0), false, SubTxIndex::GRIMM_REFUND_TX);
                SetParameter(TxParameterID::KernelUnconfirmedHeight, Height(0), false, SubTxIndex::GRIMM_REFUND_TX);

                SetState(State::SendingGrimmRefundTX);
                isRolledback = true;
            }

            if (GetParameter(TxParameterID::KernelProofHeight, proofHeight, SubTxIndex::GRIMM_LOCK_TX)
                && proofHeight > height)
            {
                SetParameter(TxParameterID::KernelProofHeight, Height(0), false, SubTxIndex::GRIMM_LOCK_TX);
                SetParameter(TxParameterID::KernelUnconfirmedHeight, Height(0), false, SubTxIndex::GRIMM_LOCK_TX);

                SetState(State::SendingGrimmLockTX);
                isRolledback = true;
            }

            return isRolledback;
        }
        else
        {
            if (GetParameter(TxParameterID::KernelProofHeight, proofHeight, SubTxIndex::GRIMM_REDEEM_TX)
                && proofHeight > height)
            {
                SetParameter(TxParameterID::KernelProofHeight, Height(0), false, SubTxIndex::GRIMM_REDEEM_TX);
                SetParameter(TxParameterID::KernelUnconfirmedHeight, Height(0), false, SubTxIndex::GRIMM_REDEEM_TX);

                SetState(State::SendingGrimmRedeemTX);
                return true;
            }
        }

        return false;
    }

    void AtomicSwapTransaction::SetNextState(State state)
    {
        SetState(state);
        UpdateAsync();
    }

    TxType AtomicSwapTransaction::GetType() const
    {
        return TxType::AtomicSwap;
    }

    AtomicSwapTransaction::State AtomicSwapTransaction::GetState(SubTxID subTxID) const
    {
        State state = State::Initial;
        GetParameter(TxParameterID::State, state, subTxID);
        return state;
    }

    AtomicSwapTransaction::SubTxState AtomicSwapTransaction::GetSubTxState(SubTxID subTxID) const
    {
        SubTxState state = SubTxState::Initial;
        GetParameter(TxParameterID::State, state, subTxID);
        return state;
    }

    Amount AtomicSwapTransaction::GetWithdrawFee() const
    {
        // TODO(alex.starun): implement fee calculation
        return kMinFeeInCentum;
    }

    void AtomicSwapTransaction::UpdateImpl()
    {
        try
        {
            CheckSubTxFailures();

            State state = GetState(kDefaultSubTxID);
            bool isGrimmOwner = IsGrimmSide();

            if (Height minHeight = 0; (state == State::Initial) && IsInitiator() && !GetParameter(TxParameterID::MinHeight, minHeight))
            {
                // init all heights
                Height currentHeight = m_WalletDB->getCurrentHeight();
                Height responseTime = GetMandatoryParameter<Height>(TxParameterID::PeerResponseHeight);
                SetParameter(TxParameterID::MinHeight, currentHeight, false);
                SetParameter(TxParameterID::PeerResponseHeight, responseTime + currentHeight);
            }

            switch (state)
            {
            case State::Initial:
            {
                if (!m_secondSide->Initialize())
                    break;

                SetNextState(State::Invitation);
                break;
            }
            case State::Invitation:
            {
                if (IsInitiator())
                {
                    m_secondSide->InitLockTime();
                    SendInvitation();
                }
                else
                {
                    if (!m_secondSide->ValidateLockTime())
                    {
                        LOG_ERROR() << GetTxID() << "[" << static_cast<SubTxID>(SubTxIndex::LOCK_TX) << "] " << "Lock height is unacceptable.";
                        OnSubTxFailed(TxFailureReason::InvalidTransaction, SubTxIndex::LOCK_TX, true);
                        break;
                    }
                }

                SetNextState(State::BuildingGrimmLockTX);
                break;
            }
            case State::BuildingGrimmLockTX:
            {
                auto lockTxState = BuildGrimmLockTx();
                if (lockTxState != SubTxState::Constructed)
                    break;
                LOG_INFO() << GetTxID() << " Grimm LockTX constructed.";
                SetNextState(State::BuildingGrimmRefundTX);
                break;
            }
            case State::BuildingGrimmRefundTX:
            {
                auto subTxState = BuildGrimmWithdrawTx(SubTxIndex::GRIMM_REFUND_TX, m_WithdrawTx);
                if (subTxState != SubTxState::Constructed)
                    break;

                m_WithdrawTx.reset();
                LOG_INFO() << GetTxID() << " Grimm RefundTX constructed.";
                SetNextState(State::BuildingGrimmRedeemTX);
                break;
            }
            case State::BuildingGrimmRedeemTX:
            {
                auto subTxState = BuildGrimmWithdrawTx(SubTxIndex::GRIMM_REDEEM_TX, m_WithdrawTx);
                if (subTxState != SubTxState::Constructed)
                    break;

                m_WithdrawTx.reset();
                LOG_INFO() << GetTxID() << " Grimm RedeemTX constructed.";
                SetNextState(State::HandlingContractTX);
                break;
            }
            case State::HandlingContractTX:
            {
                if (!isGrimmOwner)
                {
                    if (!m_secondSide->SendLockTx())
                        break;

                    SendExternalTxDetails();
                }
                else
                {
                    if (!m_secondSide->ConfirmLockTx())
                    {
                        UpdateOnNextTip();
                        break;
                    }
                }

                LOG_INFO() << GetTxID() << " LockTX completed.";
                SetNextState(State::SendingGrimmLockTX);
                break;
            }
            case State::SendingRefundTX:
            {
                assert(!isGrimmOwner);

                if (!m_secondSide->IsLockTimeExpired())
                {
                    UpdateOnNextTip();
                    break;
                }

                if (!m_secondSide->SendRefund())
                    break;

                LOG_INFO() << GetTxID() << " RefundTX completed!";
                SetNextState(State::Refunded);
                break;
            }
            case State::SendingRedeemTX:
            {
                assert(isGrimmOwner);
                if (!m_secondSide->SendRedeem())
                    break;

                LOG_INFO() << GetTxID() << " RedeemTX completed!";
                SetNextState(State::CompleteSwap);
                break;
            }
            case State::SendingGrimmLockTX:
            {
                if (!m_LockTx && isGrimmOwner)
                {
                    BuildGrimmLockTx();
                }

                if (m_LockTx && !SendSubTx(m_LockTx, SubTxIndex::GRIMM_LOCK_TX))
                    break;

                if (!isGrimmOwner && m_secondSide->IsLockTimeExpired())
                {
                    LOG_INFO() << GetTxID() << " Locktime is expired.";
                    SetNextState(State::SendingRefundTX);
                    break;
                }

                if (!CompleteSubTx(SubTxIndex::GRIMM_LOCK_TX))
                    break;

                LOG_INFO() << GetTxID() << " Grimm LockTX completed.";
                SetNextState(State::SendingGrimmRedeemTX);
                break;
            }
            case State::SendingGrimmRedeemTX:
            {
                if (isGrimmOwner)
                {
                    UpdateOnNextTip();

                    if (IsGrimmLockTimeExpired())
                    {
                        // If we already got SecretPrivateKey for RedeemTx, don't send refundTx,
                        // because it looks like we got rollback and we just should rerun TX's.
                        NoLeak<uintBig> secretPrivateKey;
                        if (!GetParameter(TxParameterID::AtomicSwapSecretPrivateKey, secretPrivateKey.V, SubTxIndex::GRIMM_REDEEM_TX))
                        {
                            LOG_INFO() << GetTxID() << " Grimm locktime expired.";
                            SetNextState(State::SendingGrimmRefundTX);
                            break;
                        }
                    }

                    // request kernel body for getting secretPrivateKey
                    if (!GetKernelFromChain(SubTxIndex::GRIMM_REDEEM_TX))
                        break;

                    ExtractSecretPrivateKey();

                    // Redeem second Coin
                    SetNextState(State::SendingRedeemTX);
                }
                else
                {
                    if (!CompleteGrimmWithdrawTx(SubTxIndex::GRIMM_REDEEM_TX))
                        break;

                    LOG_INFO() << GetTxID() << " Grimm RedeemTX completed!";
                    SetNextState(State::CompleteSwap);
                }
                break;
            }
            case State::SendingGrimmRefundTX:
            {
                assert(isGrimmOwner);
                if (!IsGrimmLockTimeExpired())
                {
                    UpdateOnNextTip();
                    break;
                }

                if (!CompleteGrimmWithdrawTx(SubTxIndex::GRIMM_REFUND_TX))
                    break;

                LOG_INFO() << GetTxID() << " Grimm Refund TX completed!";
                SetNextState(State::Refunded);
                break;
            }
            case State::CompleteSwap:
            {
                LOG_INFO() << GetTxID() << " Swap completed.";
                UpdateTxDescription(TxStatus::Completed);
                m_Gateway.on_tx_completed(GetTxID());
                break;
            }
            case State::Cancelled:
            {
                LOG_INFO() << GetTxID() << " Transaction cancelled.";
                // TODO roman.strilec: need to implement notification of counterparty
                UpdateTxDescription(TxStatus::Cancelled);

                RollbackTx();

                m_Gateway.on_tx_completed(GetTxID());
                break;
            }
            case State::Failed:
            {
                LOG_INFO() << GetTxID() << " Transaction failed.";
                UpdateTxDescription(TxStatus::Failed);
                m_Gateway.on_tx_completed(GetTxID());
                break;
            }

            case State::Refunded:
            {
                LOG_INFO() << GetTxID() << " Swap has not succeeded.";
                UpdateTxDescription(TxStatus::Completed);
                m_Gateway.on_tx_completed(GetTxID());
                break;
            }

            default:
                break;
            }
        }
        catch (const UninitilizedSecondSide&)
        {
            //LOG_ERROR() << "";
        }
    }

    void AtomicSwapTransaction::RollbackTx()
    {
        LOG_INFO() << GetTxID() << " Rollback...";

        GetWalletDB()->rollbackTx(GetTxID());
    }

    void AtomicSwapTransaction::NotifyFailure(TxFailureReason reason)
    {
        SetTxParameter msg;
        msg.AddParameter(TxParameterID::FailureReason, reason);
        SendTxParameters(std::move(msg));
    }

    void AtomicSwapTransaction::OnFailed(TxFailureReason reason, bool notify)
    {
        LOG_ERROR() << GetTxID() << " Failed. " << GetFailureMessage(reason);

        if (notify)
        {
            NotifyFailure(reason);
        }

        SetParameter(TxParameterID::InternalFailureReason, reason, false);

        State state = GetState(kDefaultSubTxID);
        bool isGrimmSide = IsGrimmSide();

        switch (state)
        {
        case State::Initial:
        case State::Invitation:
        {
            break;
        }
        case State::BuildingGrimmLockTX:
        case State::BuildingGrimmRedeemTX:
        case State::BuildingGrimmRefundTX:
        {
            RollbackTx();

            break;
        }
        case State::HandlingContractTX:
        {
            RollbackTx();

            break;
        }
        case State::SendingGrimmLockTX:
        {
            if (isGrimmSide)
            {
                RollbackTx();
                break;
            }
            else
            {
                assert(false && "Impossible case!");
                return;
            }
        }
        case State::SendingGrimmRedeemTX:
        {
            if (isGrimmSide)
            {
                assert(false && "Impossible case!");
                return;
            }
            else
            {
                SetNextState(State::SendingRefundTX);
                return;
            }
        }
        case State::SendingRedeemTX:
        {
            if (isGrimmSide)
            {
                LOG_ERROR() << "";
                return;
            }
            else
            {
                assert(false && "Impossible case!");
                return;
            }
            break;
        }
        default:
            return;
        }

        SetNextState(State::Failed);
    }

    bool AtomicSwapTransaction::CheckExpired()
    {
        if (IsGrimmSide())
        {
			uint8_t nRegistered = proto::TxStatus::Unspecified;
            if (!GetParameter(TxParameterID::TransactionRegistered, nRegistered, SubTxIndex::GRIMM_LOCK_TX))
            {
                Block::SystemState::Full state;
                Height lockTxMaxHeight = MaxHeight;

                if (GetParameter(TxParameterID::MaxHeight, lockTxMaxHeight, SubTxIndex::GRIMM_LOCK_TX) && GetTip(state) && state.m_Height > lockTxMaxHeight)
                {
                    LOG_INFO() << GetTxID() << " Transaction expired. Current height: " << state.m_Height << ", max kernel height: " << lockTxMaxHeight;
                    OnFailed(TxFailureReason::TransactionExpired, false);
                    return true;
                }
            }
        }
        return false;
    }

    bool AtomicSwapTransaction::CheckExternalFailures()
    {
        TxFailureReason reason = TxFailureReason::Unknown;
        if (GetParameter(TxParameterID::FailureReason, reason))
        {
            State state = GetState(kDefaultSubTxID);

            switch (state)
            {
            case State::Initial:
            case State::Invitation:
            {
                SetState(State::Failed);
                break;
            }
            case State::BuildingGrimmLockTX:
            case State::BuildingGrimmRedeemTX:
            case State::BuildingGrimmRefundTX:
            {
                RollbackTx();
                SetState(State::Failed);
                break;
            }
            case State::HandlingContractTX:
            {
                if (IsGrimmSide())
                {
                    RollbackTx();
                    SetState(State::Failed);
                }

                break;
            }
            case State::SendingGrimmLockTX:
            {
                // nothing
                break;
            }
            case State::SendingGrimmRedeemTX:
            {
                // nothing
                break;
            }
            case State::SendingRedeemTX:
            {
                // nothing
                break;
            }
            default:
                break;
            }
        }
        return false;
    }

    bool AtomicSwapTransaction::CompleteGrimmWithdrawTx(SubTxID subTxID)
    {
        if (!m_WithdrawTx)
        {
            BuildGrimmWithdrawTx(subTxID, m_WithdrawTx);
        }

        if (m_WithdrawTx && !SendSubTx(m_WithdrawTx, subTxID))
        {
            return false;
        }

        if (!CompleteSubTx(subTxID))
        {
            return false;
        }

        return true;
    }

    AtomicSwapTransaction::SubTxState AtomicSwapTransaction::BuildGrimmLockTx()
    {
        // load state
        SubTxState lockTxState = SubTxState::Initial;
        GetParameter(TxParameterID::State, lockTxState, SubTxIndex::GRIMM_LOCK_TX);

        bool isGrimmOwner = IsGrimmSide();
        auto fee = GetMandatoryParameter<Amount>(TxParameterID::Fee);
        auto lockTxBuilder = std::make_shared<LockTxBuilder>(*this, GetAmount(), fee);

        if (!lockTxBuilder->GetInitialTxParams() && lockTxState == SubTxState::Initial)
        {
            // TODO: check expired!

            if (isGrimmOwner)
            {
                Height maxResponseHeight = 0;
                if (GetParameter(TxParameterID::PeerResponseHeight, maxResponseHeight))
                {
                    LOG_INFO() << GetTxID() << "[" << static_cast<SubTxID>(SubTxIndex::GRIMM_LOCK_TX) << "]"
                        << " Max height for response: " << maxResponseHeight;
                }

                lockTxBuilder->SelectInputs();
                lockTxBuilder->AddChange();
            }

            UpdateTxDescription(TxStatus::InProgress);

            lockTxBuilder->GenerateOffset();
        }

        lockTxBuilder->CreateInputs();
        if (isGrimmOwner && lockTxBuilder->CreateOutputs())
        {
            return lockTxState;
        }

        lockTxBuilder->GenerateNonce();
        lockTxBuilder->LoadSharedParameters();

        if (!lockTxBuilder->UpdateMaxHeight())
        {
            OnSubTxFailed(TxFailureReason::MaxHeightIsUnacceptable, SubTxIndex::GRIMM_LOCK_TX, true);
            return lockTxState;
        }

        if (!lockTxBuilder->GetPeerPublicExcessAndNonce())
        {
            if (lockTxState == SubTxState::Initial && isGrimmOwner)
            {
                if (!IsInitiator())
                {
                    // When swap started not from Grimm side, we should save MaxHeight
                    SetParameter(TxParameterID::MaxHeight, lockTxBuilder->GetMaxHeight(), false, SubTxIndex::GRIMM_LOCK_TX);
                }

                SendLockTxInvitation(*lockTxBuilder);
                SetState(SubTxState::Invitation, SubTxIndex::GRIMM_LOCK_TX);
                lockTxState = SubTxState::Invitation;
            }
            return lockTxState;
        }

        lockTxBuilder->CreateKernel();
        lockTxBuilder->SignPartial();

        if (lockTxState == SubTxState::Initial || lockTxState == SubTxState::Invitation)
        {
            if (!lockTxBuilder->SharedUTXOProofPart2(isGrimmOwner))
            {
                return lockTxState;
            }
            SendMultiSigProofPart2(*lockTxBuilder, isGrimmOwner);
            SetState(SubTxState::SharedUTXOProofPart2, SubTxIndex::GRIMM_LOCK_TX);
            lockTxState = SubTxState::SharedUTXOProofPart2;
            return lockTxState;
        }

        if (!lockTxBuilder->GetPeerSignature())
        {
            return lockTxState;
        }

        if (!lockTxBuilder->IsPeerSignatureValid())
        {
            OnSubTxFailed(TxFailureReason::InvalidPeerSignature, SubTxIndex::GRIMM_LOCK_TX, true);
            return lockTxState;
        }

        lockTxBuilder->FinalizeSignature();

        if (lockTxState == SubTxState::SharedUTXOProofPart2)
        {
            if (!lockTxBuilder->SharedUTXOProofPart3(isGrimmOwner))
            {
                return lockTxState;
            }
            SendMultiSigProofPart3(*lockTxBuilder, isGrimmOwner);
            SetState(SubTxState::Constructed, SubTxIndex::GRIMM_LOCK_TX);
            lockTxState = SubTxState::Constructed;
        }

        if (isGrimmOwner && lockTxState == SubTxState::Constructed)
        {
            // Create TX
            auto transaction = lockTxBuilder->CreateTransaction();
            TxBase::Context::Params pars;
            TxBase::Context context(pars);
            if (!transaction->IsValid(context))
            {
                OnSubTxFailed(TxFailureReason::InvalidTransaction, SubTxIndex::GRIMM_LOCK_TX, true);
                return lockTxState;
            }

            // TODO: return
            m_LockTx = transaction;
        }

        return lockTxState;
    }

    AtomicSwapTransaction::SubTxState AtomicSwapTransaction::BuildGrimmWithdrawTx(SubTxID subTxID, Transaction::Ptr& resultTx)
    {
        SubTxState subTxState = GetSubTxState(subTxID);

        Amount withdrawFee = 0;
        Amount withdrawAmount = 0;

        if (!GetParameter(TxParameterID::Amount, withdrawAmount, subTxID) ||
            !GetParameter(TxParameterID::Fee, withdrawFee, subTxID))
        {
            withdrawFee = GetWithdrawFee();
            withdrawAmount = GetAmount() - withdrawFee;

            SetParameter(TxParameterID::Amount, withdrawAmount, subTxID);
            SetParameter(TxParameterID::Fee, withdrawFee, subTxID);
        }

        bool isTxOwner = (IsGrimmSide() && (SubTxIndex::GRIMM_REFUND_TX == subTxID)) || (!IsGrimmSide() && (SubTxIndex::GRIMM_REDEEM_TX == subTxID));
        SharedTxBuilder builder{ *this, subTxID, withdrawAmount, withdrawFee };

        if (!builder.GetSharedParameters())
        {
            return subTxState;
        }

        // send invite to get
        if (!builder.GetInitialTxParams() && subTxState == SubTxState::Initial)
        {
            builder.InitTx(isTxOwner);
        }

        builder.GenerateNonce();
        builder.CreateKernel();

        if (!builder.GetPeerPublicExcessAndNonce())
        {
            if (subTxState == SubTxState::Initial && isTxOwner)
            {
                SendSharedTxInvitation(builder);
                SetState(SubTxState::Invitation, subTxID);
                subTxState = SubTxState::Invitation;
            }
            return subTxState;
        }

        builder.SignPartial();

        if (!builder.GetPeerSignature())
        {
            if (subTxState == SubTxState::Initial && !isTxOwner)
            {
                // invited participant
                ConfirmSharedTxInvitation(builder);

                if (subTxID == SubTxIndex::GRIMM_REFUND_TX)
                {
                    SetState(SubTxState::Constructed, subTxID);
                    subTxState = SubTxState::Constructed;
                }
            }
            return subTxState;
        }

        if (subTxID == SubTxIndex::GRIMM_REDEEM_TX)
        {
            if (IsGrimmSide())
            {
                // save SecretPublicKey
                {
                    auto peerPublicNonce = GetMandatoryParameter<Point::Native>(TxParameterID::PeerPublicNonce, subTxID);
                    Scalar::Native challenge;
                    {
                        Point::Native publicNonceNative = builder.GetPublicNonce() + peerPublicNonce;
                        Point publicNonce;
                        publicNonceNative.Export(publicNonce);

                        // Signature::get_Challenge(e, m_NoncePub, msg);
                        uintBig message;
                        builder.GetKernel().get_Hash(message);

                        Oracle() << publicNonce << message >> challenge;
                    }

                    Scalar::Native peerSignature = GetMandatoryParameter<Scalar::Native>(TxParameterID::PeerSignature, subTxID);
                    auto peerPublicExcess = GetMandatoryParameter<Point::Native>(TxParameterID::PeerPublicExcess, subTxID);

                    Point::Native pt = Context::get().G * peerSignature;

                    pt += peerPublicExcess * challenge;
                    pt += peerPublicNonce;
                    assert(!(pt == Zero));

                    Point secretPublicKey;
                    pt.Export(secretPublicKey);

                    SetParameter(TxParameterID::AtomicSwapSecretPublicKey, secretPublicKey, subTxID);
                }

                SetState(SubTxState::Constructed, subTxID);
                return SubTxState::Constructed;
            }
            else
            {
                // Send BTC side partial sign with secret
                auto partialSign = builder.GetPartialSignature();
                Scalar secretPrivateKey;
                GetParameter(TxParameterID::AtomicSwapSecretPrivateKey, secretPrivateKey.m_Value, SubTxIndex::GRIMM_REDEEM_TX);
                partialSign += secretPrivateKey;

                SetTxParameter msg;
                msg.AddParameter(TxParameterID::SubTxIndex, builder.GetSubTxID())
                    .AddParameter(TxParameterID::PeerSignature, partialSign);

                if (!SendTxParameters(std::move(msg)))
                {
                    OnFailed(TxFailureReason::FailedToSendParameters, false);
                    return subTxState;
                }
            }
        }

        if (!builder.IsPeerSignatureValid())
        {
            OnSubTxFailed(TxFailureReason::InvalidPeerSignature, subTxID, true);
            return subTxState;
        }

        builder.FinalizeSignature();

        SetState(SubTxState::Constructed, subTxID);
        subTxState = SubTxState::Constructed;

        if (isTxOwner)
        {
            auto transaction = builder.CreateTransaction();
            TxBase::Context::Params pars;
            TxBase::Context context(pars);
            if (!transaction->IsValid(context))
            {
                OnSubTxFailed(TxFailureReason::InvalidTransaction, subTxID, true);
                return subTxState;
            }
            resultTx = transaction;
        }

        return subTxState;
    }

    bool AtomicSwapTransaction::SendSubTx(Transaction::Ptr transaction, SubTxID subTxID)
    {
		uint8_t nRegistered = proto::TxStatus::Unspecified;
        if (!GetParameter(TxParameterID::TransactionRegistered, nRegistered, subTxID))
        {
            m_Gateway.register_tx(GetTxID(), transaction, subTxID);
            return (proto::TxStatus::Ok == nRegistered);
        }

        if (proto::TxStatus::Ok != nRegistered)
        {
            OnSubTxFailed(TxFailureReason::FailedToRegister, subTxID, subTxID == SubTxIndex::GRIMM_LOCK_TX);
            return false;
        }

        return true;
    }

    bool AtomicSwapTransaction::IsGrimmLockTimeExpired() const
    {
        Height lockTimeHeight = MaxHeight;
        GetParameter(TxParameterID::MinHeight, lockTimeHeight);

        Block::SystemState::Full state;

        return GetTip(state) && state.m_Height > (lockTimeHeight + kGrimmLockTimeInBlocks);
    }

    bool AtomicSwapTransaction::CompleteSubTx(SubTxID subTxID)
    {
        Height hProof = 0;
        GetParameter(TxParameterID::KernelProofHeight, hProof, subTxID);
        if (!hProof)
        {
            Merkle::Hash kernelID = GetMandatoryParameter<Merkle::Hash>(TxParameterID::KernelID, subTxID);
            m_Gateway.confirm_kernel(GetTxID(), kernelID, subTxID);
            return false;
        }

        if ((SubTxIndex::GRIMM_REDEEM_TX == subTxID) || (SubTxIndex::GRIMM_REFUND_TX == subTxID))
        {
            // store Coin in DB
            auto amount = GetMandatoryParameter<Amount>(TxParameterID::Amount, subTxID);
            Coin withdrawUtxo(amount);

            withdrawUtxo.m_createTxId = GetTxID();
            withdrawUtxo.m_ID = GetMandatoryParameter<Coin::ID>(TxParameterID::SharedCoinID, subTxID);

            GetWalletDB()->save(withdrawUtxo);
        }

        std::vector<Coin> modified = GetWalletDB()->getCoinsByTx(GetTxID());
        for (auto& coin : modified)
        {
            bool bIn = (coin.m_createTxId == m_ID);
            bool bOut = (coin.m_spentTxId == m_ID);
            if (bIn || bOut)
            {
                if (bIn)
                {
                    coin.m_confirmHeight = std::min(coin.m_confirmHeight, hProof);
                    coin.m_maturity = hProof + Rules::get().Maturity.Std; // so far we don't use incubation for our created outputs
                }
                if (bOut)
                    coin.m_spentHeight = std::min(coin.m_spentHeight, hProof);
            }
        }

        GetWalletDB()->save(modified);

        return true;
    }

    bool AtomicSwapTransaction::GetKernelFromChain(SubTxID subTxID) const
    {
        Height hProof = 0;
        GetParameter(TxParameterID::KernelProofHeight, hProof, subTxID);

        if (!hProof)
        {
            Merkle::Hash kernelID = GetMandatoryParameter<Merkle::Hash>(TxParameterID::KernelID, SubTxIndex::GRIMM_REDEEM_TX);
            m_Gateway.get_kernel(GetTxID(), kernelID, subTxID);
            return false;
        }

        return true;
    }

    Amount AtomicSwapTransaction::GetAmount() const
    {
        if (!m_Amount.is_initialized())
        {
            m_Amount = GetMandatoryParameter<Amount>(TxParameterID::Amount);
        }
        return *m_Amount;
    }

    bool AtomicSwapTransaction::IsSender() const
    {
        if (!m_IsSender.is_initialized())
        {
            m_IsSender = GetMandatoryParameter<bool>(TxParameterID::IsSender);
        }
        return *m_IsSender;
    }

    bool AtomicSwapTransaction::IsGrimmSide() const
    {
        if (!m_IsGrimmSide.is_initialized())
        {
            bool isGrimmSide = false;
            GetParameter(TxParameterID::AtomicSwapIsGrimmSide, isGrimmSide);
            m_IsGrimmSide = isGrimmSide;
        }
        return *m_IsGrimmSide;
    }

    void AtomicSwapTransaction::SendInvitation()
    {
        auto swapAmount = GetMandatoryParameter<Amount>(TxParameterID::AtomicSwapAmount);
        auto swapCoin = GetMandatoryParameter<AtomicSwapCoin>(TxParameterID::AtomicSwapCoin);
        auto swapPublicKey = GetMandatoryParameter<std::string>(TxParameterID::AtomicSwapPublicKey);
        auto swapLockTime = GetMandatoryParameter<Timestamp>(TxParameterID::AtomicSwapExternalLockTime);
        auto minHeight = GetMandatoryParameter<Height>(TxParameterID::MinHeight);
        auto lifetime = GetMandatoryParameter<Height>(TxParameterID::Lifetime);

        // send invitation
        SetTxParameter msg;
        msg.AddParameter(TxParameterID::Amount, GetAmount())
            .AddParameter(TxParameterID::Fee, GetMandatoryParameter<Amount>(TxParameterID::Fee))
            .AddParameter(TxParameterID::IsSender, !IsSender())
            .AddParameter(TxParameterID::MinHeight, minHeight)
            .AddParameter(TxParameterID::Lifetime, lifetime)
            .AddParameter(TxParameterID::AtomicSwapAmount, swapAmount)
            .AddParameter(TxParameterID::AtomicSwapCoin, swapCoin)
            .AddParameter(TxParameterID::AtomicSwapPeerPublicKey, swapPublicKey)
            .AddParameter(TxParameterID::AtomicSwapExternalLockTime, swapLockTime)
            .AddParameter(TxParameterID::AtomicSwapIsGrimmSide, !IsGrimmSide())
            .AddParameter(TxParameterID::PeerProtoVersion, s_ProtoVersion);

        if (!SendTxParameters(std::move(msg)))
        {
            OnFailed(TxFailureReason::FailedToSendParameters, false);
        }
    }

    void AtomicSwapTransaction::SendExternalTxDetails()
    {
        SetTxParameter msg;
        m_secondSide->AddTxDetails(msg);

        if (!SendTxParameters(std::move(msg)))
        {
            OnFailed(TxFailureReason::FailedToSendParameters, false);
        }
    }

    void AtomicSwapTransaction::SendLockTxInvitation(const LockTxBuilder& lockBuilder)
    {
        auto swapPublicKey = GetMandatoryParameter<std::string>(TxParameterID::AtomicSwapPublicKey);

        SetTxParameter msg;
        msg.AddParameter(TxParameterID::AtomicSwapPeerPublicKey, swapPublicKey)
            .AddParameter(TxParameterID::Fee, lockBuilder.GetFee())
            .AddParameter(TxParameterID::SubTxIndex, SubTxIndex::GRIMM_LOCK_TX)
            .AddParameter(TxParameterID::PeerMaxHeight, lockBuilder.GetMaxHeight())
            .AddParameter(TxParameterID::PeerPublicExcess, lockBuilder.GetPublicExcess())
            .AddParameter(TxParameterID::PeerPublicNonce, lockBuilder.GetPublicNonce());

        if (!SendTxParameters(std::move(msg)))
        {
            OnFailed(TxFailureReason::FailedToSendParameters, false);
        }
    }

    void AtomicSwapTransaction::SendMultiSigProofPart2(const LockTxBuilder& lockBuilder, bool isMultiSigProofOwner)
    {
        SetTxParameter msg;
        msg.AddParameter(TxParameterID::SubTxIndex, SubTxIndex::GRIMM_LOCK_TX)
            .AddParameter(TxParameterID::PeerSignature, lockBuilder.GetPartialSignature())
            .AddParameter(TxParameterID::PeerOffset, lockBuilder.GetOffset())
            .AddParameter(TxParameterID::PeerPublicSharedBlindingFactor, lockBuilder.GetPublicSharedBlindingFactor());
        if (isMultiSigProofOwner)
        {
            auto proofPartialMultiSig = lockBuilder.GetProofPartialMultiSig();
            msg.AddParameter(TxParameterID::PeerSharedBulletProofMSig, proofPartialMultiSig);
        }
        else
        {
            auto bulletProof = lockBuilder.GetSharedProof();
            msg.AddParameter(TxParameterID::PeerPublicExcess, lockBuilder.GetPublicExcess())
                .AddParameter(TxParameterID::PeerPublicNonce, lockBuilder.GetPublicNonce())
                .AddParameter(TxParameterID::PeerSharedBulletProofPart2, bulletProof.m_Part2)
                .AddParameter(TxParameterID::PeerMaxHeight, lockBuilder.GetMaxHeight());
        }

        if (!SendTxParameters(std::move(msg)))
        {
            OnFailed(TxFailureReason::FailedToSendParameters, false);
        }
    }

    void AtomicSwapTransaction::SendMultiSigProofPart3(const LockTxBuilder& lockBuilder, bool isMultiSigProofOwner)
    {
        if (!isMultiSigProofOwner)
        {
            auto bulletProof = lockBuilder.GetSharedProof();
            SetTxParameter msg;
            msg.AddParameter(TxParameterID::SubTxIndex, SubTxIndex::GRIMM_LOCK_TX)
                .AddParameter(TxParameterID::PeerSharedBulletProofPart3, bulletProof.m_Part3);

            if (!SendTxParameters(std::move(msg)))
            {
                OnFailed(TxFailureReason::FailedToSendParameters, false);
            }
        }
    }

    void AtomicSwapTransaction::SendSharedTxInvitation(const BaseTxBuilder& builder)
    {
        SetTxParameter msg;
        msg.AddParameter(TxParameterID::SubTxIndex, builder.GetSubTxID())
            .AddParameter(TxParameterID::Amount, builder.GetAmount())
            .AddParameter(TxParameterID::Fee, builder.GetFee())
            .AddParameter(TxParameterID::MinHeight, builder.GetMinHeight())
            .AddParameter(TxParameterID::PeerPublicExcess, builder.GetPublicExcess())
            .AddParameter(TxParameterID::PeerPublicNonce, builder.GetPublicNonce());

        if (!SendTxParameters(std::move(msg)))
        {
            OnFailed(TxFailureReason::FailedToSendParameters, false);
        }
    }

    void AtomicSwapTransaction::ConfirmSharedTxInvitation(const BaseTxBuilder& builder)
    {
        SetTxParameter msg;
        msg.AddParameter(TxParameterID::SubTxIndex, builder.GetSubTxID())
            .AddParameter(TxParameterID::PeerPublicExcess, builder.GetPublicExcess())
            .AddParameter(TxParameterID::PeerSignature, builder.GetPartialSignature())
            .AddParameter(TxParameterID::PeerPublicNonce, builder.GetPublicNonce())
            .AddParameter(TxParameterID::PeerOffset, builder.GetOffset());

        if (!SendTxParameters(std::move(msg)))
        {
            OnFailed(TxFailureReason::FailedToSendParameters, false);
        }
    }

    void AtomicSwapTransaction::OnSubTxFailed(TxFailureReason reason, SubTxID subTxID, bool notify)
    {
        TxFailureReason previousReason;

        if (GetParameter(TxParameterID::InternalFailureReason, previousReason, subTxID) && previousReason == reason)
        {
            return;
        }

        LOG_ERROR() << GetTxID() << "[" << subTxID << "]" << " Failed. " << GetFailureMessage(reason);

        SetParameter(TxParameterID::InternalFailureReason, reason, false, subTxID);
        OnFailed(TxFailureReason::SubTxFailed, notify);
    }

    void AtomicSwapTransaction::CheckSubTxFailures()
    {
        State state = GetState(kDefaultSubTxID);
        TxFailureReason reason = TxFailureReason::Unknown;

        if ((state == State::Initial ||
            state == State::Invitation ||
            state == State::HandlingContractTX) && GetParameter(TxParameterID::InternalFailureReason, reason, SubTxIndex::LOCK_TX))
        {
            OnFailed(reason, false);
        }
    }

    void AtomicSwapTransaction::ExtractSecretPrivateKey()
    {
        auto subTxID = SubTxIndex::GRIMM_REDEEM_TX;
        TxKernel::Ptr kernel = GetMandatoryParameter<TxKernel::Ptr>(TxParameterID::Kernel, subTxID);

        SharedTxBuilder builder{ *this, subTxID };
        builder.GetSharedParameters();
        builder.GetInitialTxParams();
        builder.GetPeerPublicExcessAndNonce();
        builder.GenerateNonce();
        builder.CreateKernel();
        builder.SignPartial();

        Scalar::Native peerSignature = GetMandatoryParameter<Scalar::Native>(TxParameterID::PeerSignature, subTxID);
        Scalar::Native partialSignature = builder.GetPartialSignature();

        Scalar::Native fullSignature;
        fullSignature.Import(kernel->m_Signature.m_k);
        fullSignature = -fullSignature;
        Scalar::Native secretPrivateKeyNative = peerSignature + partialSignature;
        secretPrivateKeyNative += fullSignature;

        Scalar secretPrivateKey;
        secretPrivateKeyNative.Export(secretPrivateKey);

        SetParameter(TxParameterID::AtomicSwapSecretPrivateKey, secretPrivateKey.m_Value, false, GRIMM_REDEEM_TX);
    }

} // namespace
