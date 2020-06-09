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

#pragma once

#include "base_tx_builder.h"
#include "base_transaction.h"
#include "core/block_crypt.h"


namespace grimm::wallet
{
    class AssetTxBuilder : public BaseTxBuilder
    {
    public:
        AssetTxBuilder(BaseTransaction& tx, SubTxID subTxID, Amount fee, const AmountList& assetAmountList, AssetID assetID);

        virtual void SelectInputs();
        virtual void AddChange();
        void GenerateNewCoin(Amount amount, bool bChange);
        virtual void GenerateNewCoinList(bool bChange);
        virtual void GenerateOffset();
        virtual bool CreateOutputs();
        virtual bool CreateInputs();
        bool FinalizeOutputs();
        bool GetPeerInputsAndOutputs();
        const AmountList& GetAssetAmountList() const;

        virtual Amount GetAssetAmount() const;
        virtual AssetID GetAssetID() const { return m_AssetID; }
        virtual ECC::Point::Native GetPublicExcess() const;
        virtual Transaction::Ptr CreateTransaction();
        virtual void CreateKernel();
        virtual void SignPartial();

    protected:
        AmountList m_AssetAmountList;
        AssetID m_AssetID;
        Amount m_AssetChange;
        AssetCommand m_AssetCommand;

        TxKernel::Ptr m_AssetKernel;
        TxKernel::Ptr m_EmissionKernel;
        mutable boost::optional<Merkle::Hash> m_AssetKernelID;

        void GetSK(ECC::NoLeak<ECC::Scalar::Native> &sk) const;
        void SelectAssetInputs();
    };
}
