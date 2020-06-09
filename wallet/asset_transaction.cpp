// Copyright 2019 The Beam Team / Copyright 2019 The Grimm Team
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

#include "common.h"
#include "asset_transaction.h"
#include "utility/logger.h"

#include <boost/uuid/uuid_generators.hpp>
#include <numeric>

using namespace ECC;
using namespace std;

namespace grimm::wallet
{
    AssetTxBuilder::AssetTxBuilder(BaseTransaction &tx, SubTxID subTxID, Amount fee, const AmountList &assetAmountList, AssetID assetID)
    : BaseTxBuilder(tx, subTxID, {}, fee),
    m_AssetAmountList{assetAmountList}, m_AssetID{assetID}, m_AssetChange{0}, m_AssetCommand{AssetCommand::Zero}
{
    if (m_AssetAmountList.empty())
    {
        m_Tx.GetParameter(TxParameterID::AssetAmountList, m_AssetAmountList, m_SubTxID);
    }
    if (!m_Tx.GetParameter(TxParameterID::AssetCommand, m_AssetCommand, m_SubTxID))
    {
        LOG_WARNING() << "AssetTxBuilder defaults to Transfer";
        m_AssetCommand = AssetCommand::Transfer;
    }

}



Amount AssetTxBuilder::GetAssetAmount() const
{
    return std::accumulate(m_AssetAmountList.begin(), m_AssetAmountList.end(), 0ULL);
}

const AmountList& AssetTxBuilder::GetAssetAmountList() const
{
    return m_AssetAmountList;
}

void AssetTxBuilder::SelectInputs()
{
    LOG_INFO() << "AssetTxBuilder::SelectInputs called";
    BaseTxBuilder::SelectInputs();
    auto assetID = m_Tx.GetMandatoryParameter<AssetID>(TxParameterID::AssetID, m_SubTxID);
    if (assetID == Zero)
    {
        LOG_ERROR() << m_Tx.GetTxID() << " invalid asset ";
        throw TransactionFailedException(true, TxFailureReason::InvalidTransaction);
    }

    switch (m_AssetCommand) {
        case AssetCommand::Issue:
        {
            LOG_INFO() << m_Tx.GetTxID() << " issues asset " << GetAssetAmount() << " with asset id: " << assetID;
        }
        break;
        case AssetCommand::Transfer:
        {
            SelectAssetInputs();
        }
        break;
        case AssetCommand::Burn:
        {
            SelectAssetInputs();
        }
        break;
        default:
        {
            LOG_ERROR() << m_Tx.GetTxID() << " invalid asset command: " << int(m_AssetCommand);
            throw TransactionFailedException(true, TxFailureReason::InvalidTransaction);
        }
        break;
    }
}

void AssetTxBuilder::SelectAssetInputs()
{

    vector<Coin> coins;
    Amount assetAmount = GetAssetAmount();
    CoinIDList preselectedCoinIDs;
    
    auto selectedCoins = m_Tx.GetWalletDB()->selectCoins(assetAmount, m_AssetID);
    copy(selectedCoins.begin(), selectedCoins.end(), back_inserter(coins));

    if (coins.empty())
    {
        storage::Totals totals(*m_Tx.GetWalletDB(), m_AssetID);

        LOG_ERROR() << m_Tx.GetTxID() << "[" << m_SubTxID << "]"
                    << " You only have asset " << PrintableAmount(totals.Avail);
        throw TransactionFailedException(!m_Tx.IsInitiator(), TxFailureReason::NoInputs);
    }
    m_InputCoins.reserve(coins.size());

    Amount total = 0;
    for (auto &coin : coins)
    {
        coin.m_spentTxId = m_Tx.GetTxID();
        total += coin.m_ID.m_Value;
        m_InputCoins.push_back(Asset{coin.m_ID, coin.m_assetID, false});
        LOG_INFO() << "----------------coin.m_ID,assetID" << coin.m_ID << coin.m_assetID;
    }

    m_AssetChange += total - assetAmount;
    LOG_INFO() << "-------------------m_AssetChange" << m_AssetChange;
    m_Tx.SetParameter(TxParameterID::Change, m_AssetChange, false, m_SubTxID);
    m_Tx.SetParameter(TxParameterID::InputCoins, m_InputCoins, false, m_SubTxID);
    m_Tx.GetWalletDB()->save(coins);
}

void AssetTxBuilder::AddChange()
{
    LOG_INFO() << "AssetTxBuilder::AddChange with change: " << m_Change << " assetChange: " << m_AssetChange;
    BaseTxBuilder::AddChange();
    if (m_AssetChange == 0)
    {
        return;
    }

    GenerateNewCoin(m_AssetChange, true);
}

void AssetTxBuilder::GenerateNewCoin(Amount amount, bool bChange)
{
    LOG_INFO() << "AssetTxBuilder::GenerateNewCoin called";
    Coin newUtxo{ amount, Key::Type::Regular, m_AssetID };
    newUtxo.m_createTxId = m_Tx.GetTxID();
    if (bChange)
    {
        newUtxo.m_ID.m_Type = Key::Type::Change;
    }
    m_Tx.GetWalletDB()->store(newUtxo);

    bool bPublic = false;
    if (AssetCommand::Transfer != m_AssetCommand)
    {
        bPublic = true;
    }
    m_OutputCoins.push_back(Asset{ newUtxo.m_ID, newUtxo.m_assetID, bPublic });
    m_Tx.SetParameter(TxParameterID::OutputCoins, m_OutputCoins, false, m_SubTxID);
    for (const auto &coin : m_OutputCoins)
    {
        LOG_INFO() << "AssetTxBuilder::GenerateNewCoin result " << coin.m_AssetID;
    }
}

void AssetTxBuilder:: GenerateNewCoinList(bool bChange)
{
    BaseTxBuilder::GenerateNewCoinList(bChange);
    if (AssetCommand::Burn == m_AssetCommand)
    {
        return;
    }

    for (const auto &amount : GetAssetAmountList())
    {
        GenerateNewCoin(amount, bChange);
    }
}

void AssetTxBuilder::GenerateOffset()
{
    BaseTxBuilder::GenerateOffset();
}

bool AssetTxBuilder::CreateOutputs()
{
    return BaseTxBuilder::CreateOutputs();
}

bool AssetTxBuilder::CreateInputs()
{
    return BaseTxBuilder::CreateInputs();
}

bool AssetTxBuilder::FinalizeOutputs()
{
    return BaseTxBuilder::FinalizeOutputs();
}

bool AssetTxBuilder::GetPeerInputsAndOutputs()
{
    LOG_INFO() << "GetPeerInputsAndOutputs in asset builder";
    return BaseTxBuilder::GetPeerInputsAndOutputs();
}

ECC::Point::Native AssetTxBuilder::GetPublicExcess() const
{
    // PublicExcess = Sum(inputs) - Sum(outputs) - offset * G - (Sum(input amounts) - Sum(output amounts)) * H
    Point::Native publicAmount = Zero;
    Amount amount = 0;

    Point::Native publicAssetAmount = Zero;
    Amount assetAmount = 0;
    SwitchCommitment sc(&m_AssetID);

    for (const auto &cid : m_InputCoins)
    {
        if (cid.m_AssetID == grimm::Zero)
        {
            amount += cid.m_IDV.m_Value;
        }
        else if (cid.m_AssetID == m_AssetID)
        {
            assetAmount += cid.m_IDV.m_Value;
        }
    }
    AmountBig::AddTo(publicAmount, amount);

    Tag::AddValue(publicAssetAmount, &sc.m_hGen, assetAmount);

    amount = 0;
    publicAmount = -publicAmount;

    assetAmount = 0;
    publicAssetAmount = -publicAssetAmount;
    for (const auto &cid : m_OutputCoins)
    {
        if (cid.m_AssetID == Zero)
        {
            amount += cid.m_IDV.m_Value;
        }
        else if (cid.m_AssetID == m_AssetID)
        {
            assetAmount += cid.m_IDV.m_Value;
        }
    }
    AmountBig::AddTo(publicAmount, amount);

    Tag::AddValue(publicAssetAmount, &sc.m_hGen, assetAmount);

    Point::Native publicExcess = Context::get().G * m_Offset;
    {
        Point::Native commitment;

        for (const auto &output : m_Outputs)
        {
            if (commitment.Import(output->m_Commitment))
            {
                publicExcess += commitment;
            }
        }

        publicExcess = -publicExcess;
        for (const auto &input : m_Inputs)
        {
            if (commitment.Import(input->m_Commitment))
            {
                publicExcess += commitment;
            }
        }
    }
    publicExcess += publicAmount;

    publicExcess += publicAssetAmount;

    if (AssetCommand::Issue == m_AssetCommand || AssetCommand::Burn == m_AssetCommand)
    {
        NoLeak<Scalar::Native> sk;
        GetSK(sk);
        Point::Native pt = Context::get().G * sk.V;

        pt = -pt;

        publicExcess += pt;
    }

    return publicExcess;
}

Transaction::Ptr AssetTxBuilder::CreateTransaction()
{
    assert(m_Kernel);
    // Don't display in log infinite max height
    if (m_Kernel->m_Height.m_Max == MaxHeight)
    {
        LOG_INFO() << m_Tx.GetTxID() << "[" << m_SubTxID << "]"
                   << " Transaction created. Kernel: " << GetKernelIDString()
                   << " min height: " << m_Kernel->m_Height.m_Min;
    }
    else
    {
        LOG_INFO() << m_Tx.GetTxID() << "[" << m_SubTxID << "]"
                   << " Transaction created. Kernel: " << GetKernelIDString()
                   << " min height: " << m_Kernel->m_Height.m_Min
                   << " max height: " << m_Kernel->m_Height.m_Max;
    }

    // create transaction
    auto transaction = make_shared<Transaction>();
    if (m_EmissionKernel != nullptr) {
        LOG_INFO() << "Emission Kernel commitment " << m_EmissionKernel->m_Commitment;
        transaction->m_vKernels.push_back(move(m_EmissionKernel));
    }
    LOG_INFO() << "Kernel commitment " << m_Kernel->m_Commitment;
    transaction->m_vKernels.push_back(move(m_Kernel));
    transaction->m_Offset = m_Offset + m_PeerOffset;
    transaction->m_vInputs = move(m_Inputs);
    transaction->m_vOutputs = move(m_Outputs);
    move(m_PeerInputs.begin(), m_PeerInputs.end(), back_inserter(transaction->m_vInputs));
    move(m_PeerOutputs.begin(), m_PeerOutputs.end(), back_inserter(transaction->m_vOutputs));

    transaction->Normalize();

    return transaction;
}

void AssetTxBuilder::GetSK(NoLeak<ECC::Scalar::Native> &sk) const
{
    auto idx = m_Tx.GetMandatoryParameter<uint64_t>(TxParameterID::AssetKIDId, m_SubTxID);
    auto kid = Key::ID(idx, Key::Type::Regular);

    LOG_INFO() << "kid: " << kid.m_Idx;
    m_Tx.GetWalletDB()->get_MasterKdf()->DeriveKey(sk.V, kid);
}


void AssetTxBuilder::CreateKernel()
{
    BaseTxBuilder::CreateKernel();
    Amount assetAmount = GetAssetAmount();
    if ((AssetCommand::Issue == m_AssetCommand || AssetCommand::Burn == m_AssetCommand) && assetAmount > 0) {
        m_EmissionKernel = make_unique<TxKernel>();
        m_EmissionKernel->m_AssetEmission = (AssetCommand::Issue == m_AssetCommand) ? assetAmount : -assetAmount;
        m_EmissionKernel->m_Commitment.m_X = m_AssetID;
        m_EmissionKernel->m_Commitment.m_Y = 0;

        NoLeak<Scalar::Native> sk;
        GetSK(sk);
        m_EmissionKernel->Sign(sk.V);

        LOG_INFO() << "Created emission kernel with amount " << assetAmount << " for asset id " << m_AssetID;
    }
}

void AssetTxBuilder::SignPartial()
{
    // create signature
    Point::Native totalPublicExcess = GetPublicExcess();
    totalPublicExcess += m_PeerPublicExcess;
    m_Kernel->m_Commitment = totalPublicExcess;

    m_Kernel->get_Hash(m_Message, m_PeerLockImage.get());

    auto offset = m_Offset;
    if (AssetCommand::Issue == m_AssetCommand || AssetCommand::Burn == m_AssetCommand)
    {
        NoLeak<Scalar::Native> sk;
        GetSK(sk);
        offset += sk.V;
    }
    m_PartialSignature = m_Tx.GetKeyKeeper()->SignSync(m_InputCoins, m_OutputCoins, offset, m_NonceSlot, m_Message, GetPublicNonce() + m_PeerPublicNonce, totalPublicExcess);

    StoreKernelID();
}

} // namespace grimm::wallet
