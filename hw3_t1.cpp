/* Yonathan Fisseha, yf2ey
 * Creates a dot file for 'for' loops and if statements. Credit to Eli Bendersky
 * for the 'if' part of the code. I have added the part for 'for' loops per the
 * assignment instruction.*/

#include <sstream>
#include <string>

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
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

vector<IfStmt *> ifstmtvector;
vector<ForStmt *> forstmtvector;
std::map<FunctionDecl, vector<FunctionDecl>> functionDecl;
vector<Stmt *> stmtvector;
vector<FunctionDecl> functiondeclvector;
vector<Stmt *> callexprvector;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitStmt(Stmt *st) {
    // Only function definitions (with bodies), not declarations.
    if (CallExpr *call = dyn_cast<CallExpr>(st)) {
      callexprvector.push_back(st);
    }
    return true;
  }

private:
  Rewriter &TheRewriter;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      //(*b)->dump();
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};

const clang::Decl *get_DeclContext_from_Stmt(const clang::Stmt &stmt,
                                             ASTContext *context) {
  auto it = context->getParents(stmt).begin();

  if (it == context->getParents(stmt).end())
    return nullptr;

  const clang::Decl *aDecl = it->get<clang::Decl>();
  if (aDecl && isa<FunctionDecl>(aDecl))
    return aDecl;

  const clang::Stmt *aStmt = it->get<clang::Stmt>();
  if (aStmt)
    return get_DeclContext_from_Stmt(*aStmt, context);

  return nullptr;
}

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
    llvm::errs() << "** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";

    ofstream myfile;
    myfile.open("call_graph.dot");
    cout << ("call_graph.dot") << endl;
    myfile << "digraph unnamed {\n";

    for (unsigned exprid = 0; exprid < callexprvector.size(); exprid++) {
      const CallExpr *callee =
          dyn_cast<CallExpr>(callexprvector[exprid]); //*stmt
      const FunctionDecl *caller = dyn_cast<FunctionDecl>(
          get_DeclContext_from_Stmt(*callexprvector[exprid], Context)); //*decl

      string callerStr = caller->getNameInfo().getAsString();
      string calleeStr = callee->getDirectCallee()->getNameInfo().getAsString();

      myfile << callerStr << "[shape=record,label=\"" << callerStr << "\"];\n";
      myfile << calleeStr << "[shape=record,label=\"" << calleeStr << "\"];\n";

      myfile << callerStr << " -> " << calleeStr << ";\n";
    }
    /*
        myfile << "Node1 [shape=record,label=\"{ [(ENTRY)]\\l}\"];\n";

        myfile << "Node2 [shape=record,label=\"Dummy Node\"]\n";
        myfile << "Node1 -> Node2;\n";
    */
    myfile << "}\n";
    myfile.close();
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    llvm::errs() << "** Creating AST consumer for: " << file << "\n";
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    Context = &CI.getASTContext();
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
  ASTContext *Context;
};

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  // ClangTool::run accepts a FrontendActionFactory, which is then used to
  // create new objects implementing the FrontendAction interface. Here we use
  // the helper newFrontendActionFactory to create a default factory that will
  // return a new MyFrontendAction object every time.
  // To further customize this, we could create our own factory class.
  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
