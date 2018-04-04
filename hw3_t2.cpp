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
  int Lock;
  int MyMalloc;

  ProgramPairState(int lock, int myMalloc) : Lock(lock), MyMalloc(myMalloc) {}
  bool operator==(const ProgramPairState &X) const {
    return Lock == X.Lock && MyMalloc == X.MyMalloc;
  }

  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(Lock);
    ID.AddInteger(MyMalloc);
  }
};

class PairFunctionChecker : public Checker<check::PreCall, check::PostCall> {
public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
};

REGISTER_MAP_WITH_PROGRAMSTATE(PairMap, const Decl *, ProgramPairState)

void PairFunctionChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {

  ProgramStateRef state = C.getState();
  auto caller = C.getCurrentAnalysisDeclContext()->getDecl();
  auto callee = Call.getDecl();

  // current state
  const ProgramPairState *myState = state->get<PairMap>(caller);

  if (myState == NULL) {
    ProgramPairState emptyState(0, 0);
    state = state->set<PairMap>(caller, emptyState);
    // myState = &emptyState; // this line fixes the null ptr problem
    // by forcing the state to have a value however, the state will obviously be
    // lost
  }

  if (Call.isGlobalCFunction("lock")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(myState->Lock + 1, myState->MyMalloc));
  }

  if (Call.isGlobalCFunction("unlock")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(myState->Lock - 1, myState->MyMalloc));

    if (state->get<PairMap>(caller)->Lock < 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  if (Call.isGlobalCFunction("mymalloc")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(myState->Lock, 1 + myState->MyMalloc));
  }

  if (Call.isGlobalCFunction("myfree")) {
    state = state->set<PairMap>(
        caller, ProgramPairState(myState->Lock, myState->MyMalloc - 1));

    if (state->get<PairMap>(caller)->MyMalloc < 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
  }

  C.addTransition(state);
}

void PairFunctionChecker::checkPostCall(const CallEvent &Call,
                                        CheckerContext &C) const {

  ProgramStateRef state = C.getState();
  const Decl *caller = C.getCurrentAnalysisDeclContext()->getDecl();
  auto callee = Call.getDecl();

  const ProgramPairState *myState = state->get<PairMap>(caller);

  if (myState == NULL) {
    state->set<PairMap>(caller, ProgramPairState(0, 0));
    C.addTransition(state);
    myState = state->get<PairMap>(caller);
    // myState = &emptyState; // this line fixes the null ptr problem
    // by forcing the state to have a value however, the state will obviously be
    // lost return;
  }
  if (Call.isGlobalCFunction("lock")) {
    if (state->get<PairMap>(caller)->Lock != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
    else {
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": True";
    }
  }

  if (Call.isGlobalCFunction("unlock")) {
    if (state->get<PairMap>(caller)->Lock != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";
    else {
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": True";
    }
  }

  if (Call.isGlobalCFunction("mymalloc")) {
    if (state->get<PairMap>(caller)->MyMalloc != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";

    else {
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": True";
    }
  }

  if (Call.isGlobalCFunction("myfree")) {
    if (state->get<PairMap>(caller)->MyMalloc != 0)
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": False";

    else {
      llvm::outs() << caller->getAsFunction()->getNameInfo().getAsString()
                   << ": True";
    }
  }
  C.addTransition(state);
}

void ento::registerPairFunctionChecker(CheckerManager &mgr) {
  mgr.registerChecker<PairFunctionChecker>();
}
