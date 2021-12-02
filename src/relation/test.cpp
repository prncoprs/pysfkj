#include "relation.h"

auto read_test_options(int32_t argcp, char **argvp) {
  namespace po = boost::program_options;
  ENCRYPTO::PsiAnalyticsContext context;
  po::options_description allowed("Allowed options");
  std::string type;
  // clang-format off
  allowed.add_options()("help,h", "produce this message")
  ("role,r",         po::value<decltype(context.role)>(&context.role)->required(),                                  "Role of the node")
  ("bit-length,b",   po::value<decltype(context.bitlen)>(&context.bitlen)->default_value(61u),                      "Bit-length of the elements")
  ("epsilon,e",      po::value<decltype(context.epsilon)>(&context.epsilon)->default_value(1.27f),                   "Epsilon, a table size multiplier")
  ("address,a",      po::value<decltype(context.address)>(&context.address)->default_value("127.0.0.1"),            "IP address of the server")
  ("port,p",         po::value<decltype(context.port)>(&context.port)->default_value(7777),                         "Port of the server")
  ("threshold,c",    po::value<decltype(context.threshold)>(&context.threshold)->default_value(0u),                 "Show PSI size if it is > threshold")
  ("nmegabins,m",    po::value<decltype(context.nmegabins)>(&context.nmegabins)->default_value(1u),                 "Number of mega bins")
  ("functions,f",    po::value<decltype(context.nfuns)>(&context.nfuns)->default_value(3u),                         "Number of hash functions in hash tables")
  ("threads,t",      po::value<decltype(context.nthreads)>(&context.nthreads)->default_value(1),                    "Number of threads");  // clang-format on

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argcp, argvp, allowed), vm);
    po::notify(vm);
  } catch (const boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<
               boost::program_options::required_option> > &e) {
    if (!vm.count("help")) {
      std::cout << e.what() << std::endl;
      std::cout << allowed << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (vm.count("help")) {
    std::cout << allowed << "\n";
    exit(EXIT_SUCCESS);
  }
  return context;
}

void CheckPhase(vector<vector<uint32_t>> outputs, vector<bool> eqtags, ENCRYPTO::PsiAnalyticsContext &context) {
  cout << "check final result" << endl;
    std::unique_ptr<CSocket> sockres = ENCRYPTO::EstablishConnection(context.address, context.port, static_cast<e_role>(context.role));
    if (context.role == SERVER) {
      std::vector<uint32_t> receive(outputs.size() * (outputs[0].size() + 1));
      sockres->Receive(receive.data(), receive.size() * sizeof(uint32_t));
      sockres->Close();
      for (auto i=0, j=0; i<outputs.size(); ++i) {
        bool tag = receive[j++];
        cout << (eqtags[i] ^ tag) << " : ";
        for (auto k=0; k<outputs[i].size(); ++k) {
          cout << ((uint32_t)(outputs[i][k] + receive[j])) << '|';
          j++;
        }
        cout << endl;
      }
    } else {
      std::vector<uint32_t> send;
      for (auto i=0; i<outputs.size(); ++i) {
        send.push_back(eqtags[i]);
        for (auto j=0; j<outputs[i].size(); ++j) {
          send.push_back(outputs[i][j]);
        }
      }
      sockres->Send(send.data(), send.size() * sizeof(uint32_t));
      sockres->Close();
    }
}


void testsharedjoinwithtags(ENCRYPTO::PsiAnalyticsContext config) {

  std::vector<uint32_t> inputid = {0};
  std::vector<uint32_t> inputid2 = {2};
  std::vector<std::vector<uint32_t>> Aweights, Bweights;
  std::vector<std::vector<uint32_t>> outputs;
  std::vector<bool> BTags, equaltags;

  if (config.role == SERVER) {
    Aweights = {{1,11,111},{2,22,222},{3,33,333},{4,44,444},{5,55,555},{6,66,666},{7,77,777}};
    Bweights = {{0, 1000}, {0, 2000}, {0, 3000}, {0, 4000}};
    BTags = {0, 0, 1, 1};
    SharedJoinWithTagServer(inputid, Aweights, Bweights, BTags, outputs, equaltags, config);
  } else {
    Aweights = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};
    Bweights = {{1,10}, {2,20}, {3,30}, {4, 40}};
    BTags = {0, 1, 0, 1};
    SharedJoinWithTagClient(inputid, Bweights, Aweights, BTags, outputs, equaltags, config);
  }

  for (auto i=0; i<outputs.size(); ++i) {
    for (auto j=0; j<outputs[i].size(); ++j) {
      cout << outputs[i][j] << ' ';
    }
    cout << endl;
  }
  CheckPhase(outputs, equaltags, config);
}


int main(int argc, char **argv) {
    auto config = read_test_options(argc, argv);

    Relation A, B, C;

    uint32_t AID = 2;
    uint32_t BID = 0;
    std::vector<std::vector<uint32_t>> Atuples, Btuples;

    Atuples = {{1, 11, 2}, {2, 22, 2}, {3, 33, 1}, {4, 44, 7}, {5, 55, 9}, {6, 66, 3}, {7, 77, 1}, {8, 88, 1}};  
    Btuples = {{1,10}, {2,20}, {3,30}, {4, 40}, {5, 50}};

    std::vector<bool> at(Atuples.size(), true);
    std::vector<bool> bt(Btuples.size(), true);
    std::vector<bool> bt0(Btuples.size(), false);

    A.SetOwner(0);
    A.SetType(0);
    B.SetOwner(1);
    B.SetType(0);
    A.SetTable(Atuples);
    A.SetTags(at);
    B.SetTable(Btuples);
    B.SetTags(bt);
    if (config.role == SERVER) {
        B.SetTags(bt0);
    }

    A.Join(B, C, AID, BID, config);

    CheckPhase(C.getTables(), C.getTags(), config);

    return 0;
}
