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
vector<CallExpr *> callexprvector;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

// get the source code of the specific parts of AST
template <typename T>
static std::string getText(const SourceManager &SourceManager, const T &Node) {
  SourceLocation StartSpellingLocation =
      SourceManager.getSpellingLoc(Node.getLocStart());
  SourceLocation EndSpellingLocation =
      SourceManager.getSpellingLoc(Node.getLocEnd());
  if (!StartSpellingLocation.isValid() || !EndSpellingLocation.isValid()) {
    return std::string();
  }
  bool Invalid = true;
  const char *Text =
      SourceManager.getCharacterData(StartSpellingLocation, &Invalid);
  if (Invalid) {
    return std::string();
  }
  std::pair<FileID, unsigned> Start =
      SourceManager.getDecomposedLoc(StartSpellingLocation);
  std::pair<FileID, unsigned> End =
      SourceManager.getDecomposedLoc(Lexer::getLocForEndOfToken(
          EndSpellingLocation, 0, SourceManager, LangOptions()));
  if (Start.first != End.first) {
    // Start and end are in different files.
    return std::string();
  }
  if (End.second < Start.second) {
    // Shuffling text with macros may cause this.
    return std::string();
  }
  return std::string(Text, End.second - Start.second);
}

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitStmt(Stmt *st) {
    // Only function definitions (with bodies), not declarations.
    if (CallExpr *call = dyn_cast<CallExpr>(st)) {
      callexprvector.push_back(call);
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

const clang::Decl *get_DeclContext_from_Stmt(const clang::Stmt &stmt) {
  auto it = ASTContext->getParents(stmt).begin();

  if (it == ASTContext->getParents(stmt).end())
    return nullptr;

  const clang::Decl *aDecl = it->get<clang::Decl>();
  if (aDecl)
    return aDecl;

  const clang::Stmt *aStmt = it->get<clang::Stmt>();
  if (aStmt)
    return get_DeclContext_from_Stmt(*aStmt);

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

    // handle each function definition
    for (unsigned exprid = 0; exprid < callexprvector.size(); exprid++) {
      ofstream myfile;
      myfile.open("call_graph.dot");
      cout << ("call_graph.dot") << endl;
      myfile << "digraph unnamed {\n";

      while (true) {
        // get parents
        const auto &parents = Context->getParents(*callexprvector[exprid]);
        if (parents.empty()) {
          llvm::errs() << "Can not find parent\n";
        }
        llvm::errs() << "find parent size=" << parents.size() << "\n";

        for (int j = 0; j < parents.size(); j++) {
          const Stmt *ST = parents[j].get<Stmt>();

          //	if (FunctionDecl *decl= dyn_cast<FunctionDec>(st)) {

          if (ISA(FunctionDecl, ST)) {
          }
          // const clang::Decl *aDecl = parents->get<clang::Decl>();
          //	  if(aDecl)
        }
      }

      myfile << "Node1 [shape=record,label=\"{ [(ENTRY)]\\l}\"];\n";

      myfile << "Node2 [shape=record,label=\"Dummy Node\"]\n";
      myfile << "Node1 -> Node2;\n";

      myfile << "}\n";
      myfile.close();
    }
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
