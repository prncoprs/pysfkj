#include "relation.h"

void Relation::Join(Relation B, Relation &C, uint32_t AID, uint32_t BID, ENCRYPTO::PsiAnalyticsContext config) {
    uint32_t Aowner = _owner;
    uint32_t Bowner = B._owner;

    if (Aowner == Bowner) {
        // plain text join program
        return;
    } else {
        vector< vector<uint32_t> > tableA = _table;
        vector< vector<uint32_t> > tableB = B.getTables();
        vector< vector<uint32_t> > tableC;
        vector< bool > Ctags;

        ENCRYPTO::PsiAnalyticsContext roleconfig = config;

        cout << "enter join" << endl;

        if (Aowner == config.role) { // role == SERVER
            roleconfig.role = SERVER;
            SharedJoinWithTagServer({AID}, tableA, tableB, B.getTags(), tableC, Ctags, roleconfig);
        } else { // role == CLIENT
            roleconfig.role = CLIENT;
            SharedJoinWithTagClient({BID}, tableB, tableA, B.getTags(), tableC, Ctags, roleconfig);
        }
        C.SetOwner(_owner);
        C.SetType(_type);
        C.SetTable(tableC);
        C.SetTags(Ctags);
    }
}