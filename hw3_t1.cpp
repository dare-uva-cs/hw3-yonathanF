/* Yonathan Fisseha, yf2ey
 * Creates a dot file for 'for' loops and if statements. Credit to Eli Bendersky
 * for the 'if' part of the code. I have added the part for 'for' loops per the
 * assignment instruction.*/

#include <sstream>
#include <string>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::ast_matchers;

class FunctionPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {

    auto callee = Result.Nodes.getNodeAs<clang::CallExpr>("callee");
    auto caller = Result.Nodes.getNodeAs<clang::CallExpr>("caller");

    std::string callee_str =
        callee->getDirectCallee()->getNameInfo().getName().getAsString();

    std::string caller_str =
        caller->getDirectCallee()->getNameInfo().getName().getAsString();
    std::cout << callee_str << std::endl;
    /*

   std::cout << caller_str << std::endl;
   */
  }
};

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  MatchFinder Finder;
  FunctionPrinter Printer;
  /* Finder.addMatcher(forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(
                                 hasInitializer(integerLiteral(equals(0))))))))
                         .bind("forLoop"),
                     &Printer);
 */

  Finder.addMatcher(callExpr(callee(functionDecl()),
                             hasAncestor(functionDecl().bind("caller")))
                        .bind("callee"),
                    &Printer);
  /*
    Finder.addMatcher(callExpr(callee(hasDescendant(declRefExpr(
                                   to(functionDecl().bind("callee"))))))
                          .bind("caller"));
  */
  return Tool.run(newFrontendActionFactory(&Finder).get());
}
