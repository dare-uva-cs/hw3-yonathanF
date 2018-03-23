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
private:
  enum Kind { Opened, Closed } K;
  ProgramPairState(Kind Ink) : K(Ink) {}

public:
  bool operator==(const ProgramPairState &X) const { return K == X.k; }

  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddInteger(K); }

  bool isOpened() const { return K == Opened; }
  bool isClosed() const { return K == Closed; }

  static ProgramPairState getOpened() { return ProgramPairState(Opened); }
  static ProgramPairState getClosed() { return ProgramPairSate(Closed); }
};

namespace {
class PairFunctionChecker : public Checker<check::PreCall, check::PostCall> {
public:
  CallDescription OpenFn, CloseFn;
  bool guaranteedNotToClose(const CallEvent &Call) const;

public:
  PairFunctionChecker();
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;

  /// Stop tracking addresses which escape.
  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols &Escaped,
                                     const CallEvent *Call,
                                     PointerEscapeKind Kind) const;
};
} // namespace

REGISTER_MAP_WITH_PROGRAMSTATE(PairMap, Decl *, ProgramPairState)

void PairFunctionChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {
  unsigned Indentation = 0;
  for (const LocationContext *LC = C.getLocationContext()->getParent();
       LC != nullptr; LC = LC->getParent())
    ++Indentation;

  // It is mildly evil to print directly to llvm::outs() rather than emitting
  // warnings, but this ensures things do not get filtered out by the rest of
  // the static analyzer machinery.
  llvm::outs().indent(Indentation);
  Call.dump(llvm::outs());
}

void CallDumper::checkPostCall(const CallEvent &Call, CheckerContext &C) const {

  if(Call.isCalled("unlock"){}
	
  if (Call.isCalled("myfree"){}


  if (Call.getResultType()->isVoidType())
    llvm::outs() << "Returning void\n";
  else
    llvm::outs() << "Returning " << C.getSVal(CallE) << "\n";
}

void ento::registerCallDumper(CheckerManager &mgr) {
  mgr.registerChecker<CallDumper>();
}
