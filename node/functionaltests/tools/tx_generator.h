// Copyright 2018 The Beam Team / Copyright 2019 The Grimm Team
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

#include "node/node.h"

class TxGenerator
{
public:
	using Inputs = std::vector<grimm::Input>;
public:
	TxGenerator(grimm::Key::IKdf& kdf);

	void GenerateInputInTx(grimm::Height h, grimm::Amount v, grimm::Key::Type keyType = grimm::Key::Type::Coinbase, uint32_t ind = 0);
	void GenerateOutputInTx(grimm::Height h, grimm::Amount v, grimm::Key::Type keyType = grimm::Key::Type::Regular, bool isPublic = false, uint32_t ind = 0);
	void GenerateKernel(grimm::Height h, grimm::Amount fee = 0, uint32_t ind = 0);
	void GenerateKernel();

	const grimm::proto::NewTransaction& GetTransaction();
	bool IsValid() const;

	void Sort();
	void SortInputs();
	void SortOutputs();
	void SortKernels();

	void ZeroOffset();

	Inputs GenerateInputsFromOutputs();

private:
	grimm::Key::IKdf& m_Kdf;
	grimm::proto::NewTransaction m_MsgTx;
	ECC::Scalar::Native m_Offset;
};
