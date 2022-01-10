#include "relation_wrapper.h"

namespace py = pybind11;

void CheckFinalResult(vector<vector<uint32_t>> outputs, vector<bool> eqtags,
                      ENCRYPTO::PsiAnalyticsContext &context) {
  cout << "check final result" << endl;
  std::unique_ptr<CSocket> sockres = ENCRYPTO::EstablishConnection(
      context.address, context.port, static_cast<e_role>(context.role));
  if (context.role == SERVER) {
    std::vector<uint32_t> receive(outputs.size() * (outputs[0].size() + 1));
    sockres->Receive(receive.data(), receive.size() * sizeof(uint32_t));
    sockres->Close();
    for (auto i = 0, j = 0; i < outputs.size(); ++i) {
      bool tag = receive[j++];
      cout << (eqtags[i] ^ tag) << " : ";
      for (auto k = 0; k < outputs[i].size(); ++k) {
        cout << ((uint32_t)(outputs[i][k] + receive[j])) << '|';
        j++;
      }
      cout << endl;
    }
  } else {
    std::vector<uint32_t> send;
    for (auto i = 0; i < outputs.size(); ++i) {
      send.push_back(eqtags[i]);
      for (auto j = 0; j < outputs[i].size(); ++j) {
        send.push_back(outputs[i][j]);
      }
    }
    sockres->Send(send.data(), send.size() * sizeof(uint32_t));
    sockres->Close();
  }
}

PYBIND11_MODULE(pysfkj, m) {
  py::class_<Relation>(m, "Relation")
      .def(py::init<>())
      .def("SetOwner", &Relation::SetOwner, py::arg("owner"))
      .def("SetType", &Relation::SetType, py::arg("type"))
      .def("SetTable", &Relation::SetTable, py::arg("table"))
      .def("SetTags", &Relation::SetTags, py::arg("tags"))
      .def("getTags", &Relation::getTags)
      .def("getTables", &Relation::getTables)
      .def("Join", &Relation::Join, py::arg("B"), py::arg("C"), py::arg("AID"), py::arg("BID"),
           py::arg("config"));

  py::class_<ENCRYPTO::PsiAnalyticsContext>(m, "PsiAnaCtx")
      .def(py::init<>())
      .def_readwrite("role", &ENCRYPTO::PsiAnalyticsContext::role)
      .def_readwrite("bitlen", &ENCRYPTO::PsiAnalyticsContext::bitlen)
      .def_readwrite("epsilon", &ENCRYPTO::PsiAnalyticsContext::epsilon)
      .def_readwrite("address", &ENCRYPTO::PsiAnalyticsContext::address)
      .def_readwrite("port", &ENCRYPTO::PsiAnalyticsContext::port)
      .def_readwrite("threshold", &ENCRYPTO::PsiAnalyticsContext::threshold)
      .def_readwrite("nmegabins", &ENCRYPTO::PsiAnalyticsContext::nmegabins)
      .def_readwrite("nfuns", &ENCRYPTO::PsiAnalyticsContext::nfuns)
      .def_readwrite("nthreads", &ENCRYPTO::PsiAnalyticsContext::nthreads);

  m.def("CheckFinalResult", &CheckFinalResult, py::arg("outputs"), py::arg("eqtags"),
        py::arg("config"));
}