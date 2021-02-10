// Copyright 2019 The Grimm Team
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

#include "default_peers.h"

namespace grimm
{
    std::vector<std::string> getDefaultPeers()
    {
        std::vector<std::string> result
        {
#ifdef XGM_TESTNET

            "159.65.53.233:10000",
            "159.65.61.159:10000"

#else
         //   "64.225.32.192:10150",
         //   "167.172.32.97:10150",
            "18.195.147.250:10000",
            "157.245.71.80:10150"
          //  "134.209.89.56:8385",
          //  "157.245.71.80:8385"
#endif
        };

        return result;
    }
}
