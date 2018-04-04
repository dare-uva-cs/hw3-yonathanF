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

#include <vector>

using namespace clang;
using namespace ento;
using namespace std;

int pathCounter = 0;
const Decl *mainFun = NULL;

// create a state specifier
struct PathState {
public:
  vector<const Decl *> vect;
  PathState(vector<const Decl *> Vect) : vect(Vect) {}

  bool operator==(const PathState &X) const { return vect == X.vect; }

  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddInteger(vect.size()); }
};

class PathChecker : public Checker<check::PreCall, check::PostCall> {
public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreStmt(const ReturnStmt *S, CheckerContext &C) const;
};

REGISTER_MAP_WITH_PROGRAMSTATE(PairMap, const Decl *, PathState)
void PathChecker::checkPostCall(const CallEvent &Call,
                                CheckerContext &C) const {}

void PathChecker::checkPreCall(const CallEvent &Call, CheckerContext &C) const {

  ProgramStateRef state = C.getState();
  auto caller = C.getCurrentAnalysisDeclContext()->getDecl();
  auto callee = Call.getDecl();

  if (caller->getAsFunction()->getNameInfo().getAsString().compare("main") ==
      0) {
    mainFun = caller;
    vector<const Decl *> vec;
    state->set<PairMap>(mainFun, PathState(vec));
  }

  else {
    const PathState *myState = state->get<PairMap>(mainFun);
    vector<const Decl *> vectTemp = myState->vect;
    vectTemp.push_back(callee);
    state->set<PairMap>(mainFun, PathState(vectTemp));
  }

  C.addTransition(state);
}
void PathChecker::checkPreStmt(const ReturnStmt *S, CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  auto caller = C.getCurrentAnalysisDeclContext()->getDecl();

  // current state
  const PathState *myState = state->get<PairMap>(mainFun);

  if (myState == NULL) {
    return;
  }

  if (caller->getAsFunction()->getNameInfo().getAsString().compare("main") ==
      0) {
    auto vect = myState->vect;

    llvm::outs() << "P" << pathCounter << ": ";
    for (int i = 0; i < vect.size(); i++)
      llvm::outs() << vect[i]->getAsFunction()->getNameInfo().getAsString()
                   << " ";

    pathCounter++;
    llvm::outs() << "\n";
  }
}
void ento::registerPathChecker(CheckerManager &mgr) {
  mgr.registerChecker<PathChecker>();
}
