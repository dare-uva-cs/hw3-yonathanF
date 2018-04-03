//== TraversalChecker.cpp -------------------------------------- -*- C++ -*--=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// These checkers print various aspects of the ExprEngine's traversal of the CFG
// as it builds the ExplodedGraph.
//
//===----------------------------------------------------------------------===//
#include "ClangSACheckers.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/StmtObjC.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;

// create a state specifier
struct ProgramPairState {

public:
  vector<Decl *> declVector;

  ProgramPairState(int lock, int myMalloc) : Lock(lock), MyMalloc(myMalloc) {}
  bool operator==(const ProgramPairState &X) const {
    return declVector == declVector;
  }

  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(Lock);
    ID.AddInteger(MyMalloc);
  }
};

namespace {
class PairFunctionChecker : public Checker<check::PreCall, check::PostCall> {
public:
  PairFunctionChecker();
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
};
} // namespace

REGISTER_MAP_WITH_PROGRAMSTATE(PairMap, Decl *, ProgramPairState)

void PairFunctionChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {

  ProgramStateRef State = C.getState();
  auto caller = C.getCurrentAnalysisDeclContext()->getDecl();
  auto callee = Call.getDecl();

  // current state
  ProgramPairState myState = state->get<PairMap>(caller);

  if (Call.isCalled("lock")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(++myState.Lock, myState.MyMalloc));
  }

  if (Call.isCalled("unlock")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(--myState.Lock, myState.MyMalloc));
    if (state->get<PairMap>(caller).Lock < 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  if (Call.isCalled("mymalloc")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(myState.Lock, ++myState.MyMalloc));
  }

  if (Call.isCalled("myfree")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(myState.Lock, --myState.MyMalloc));

    if (state->get<PairMap>(caller).MyMalloc < 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  C.addTransition(state);
}

void CallDumper::checkPostCall(const CallEvent &Call, CheckerContext &C) const {

  ProgramStateRef State = C.getState();
  auto caller = C.getCurrentAnalysisDeclContext()->getDecl();
  auto callee = Call.getDecl();

  ProgramPairState myState = state->get<PairMap>(caller);

  if (Call.isCalled("lock")) {
    if (state->get<PairMap>(caller).Lock != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  if (Call.isCalled("unlock")) {
    if (state->get<PairMap>(caller).Lock != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  if (Call.isCalled("mymalloc")) {
    if (state->get<PairMap>(caller).MyMalloc != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  if (Call.isCalled("myfree")) {
    if (state->get<PairMap>(caller).MyMalloc != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }
  C.addTransition(state);
}

void ento::registerCallDumper(CheckerManager &mgr) {
  mgr.registerChecker<CallDumper>();
}
