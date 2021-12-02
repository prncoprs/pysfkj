#pragma once
#include <iostream>
#include <cstdio>
#include <vector>

#include <cassert>
#include <iostream>
#include <unistd.h>

#include <boost/program_options.hpp>

#include "ENCRYPTO_utils/connection.h"
#include "ENCRYPTO_utils/socket.h"
#include "ENCRYPTO_utils/crypto/crypto.h"
#include <ENCRYPTO_utils/parse_options.h>
#include "abycore/aby/abyparty.h"

#include "common/psi_analytics.h"
#include "common/psi_analytics_context.h"
#include "OEP/OEP.h"
#include "join/join.h"
#include "utils/MurmurHash3.h"
#include "Purify/PurificationCircuit.h"

using namespace std;

class Relation {
    public:
        void SetOwner(uint32_t owner) {
            _owner = owner;
        }
        void SetType(uint32_t type) {
            _type = type;
        }
        void SetTable(vector< vector<uint32_t> > table) {
            _table.resize(table.size());
            for (auto i=0; i<table.size(); ++i) {
                _table[i] = table[i];
            }
        }
        void SetTags(vector< bool > tags) {
            _tags.resize(tags.size());
            for (auto i=0; i<tags.size(); ++i) {
                _tags[i] = tags[i];
            }
        }
        vector< bool > getTags() {
            return _tags;
        }

        vector<vector<uint32_t>> getTables() {
            return _table;
        }

        void Join(Relation B, Relation &C, uint32_t AID, uint32_t BID, ENCRYPTO::PsiAnalyticsContext config); // C = A join B

    private:
        uint32_t _owner; // 0 - Alice ; 1 - Bob
        uint32_t _type; // 0 - plain ; 1 - shared
        vector< vector<uint32_t> > _table;
        vector< bool > _tags;
};
