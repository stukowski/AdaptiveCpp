/*
 * This file is part of hipSYCL, a SYCL implementation based on CUDA/HIP
 *
 * Copyright (c) 2018 Aksel Alpay
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sstream>
#include <string>
#include <boost/preprocessor/stringize.hpp>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/Version.h"
#include "llvm/Support/raw_ostream.h"

#include "HipsyclTransform.hpp"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;



// For each source file provided to the tool, a new FrontendAction is created.


int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, llvm::cl::GeneralCategory);

  ClangTool tool(op.getCompilations(), op.getSourcePathList());

  ArgumentsAdjuster adjuster =
      [](const CommandLineArguments& args,StringRef) -> CommandLineArguments
  {
    CommandLineArguments modifiedArgs = args;

    modifiedArgs.push_back("-D__global__=__attribute__((target(\"kernel\")))");
    modifiedArgs.push_back("-D__host__=__attribute__((target(\"host\")))");
    modifiedArgs.push_back("-D__device__=__attribute__((target(\"device\")))");
    //modifiedArgs.push_back("-D__constant__");
    //modifiedArgs.push_back("-D__shared__");

    modifiedArgs.push_back("-D__HIPSYCL_TRANSFORM__");

    modifiedArgs.push_back("-Wno-ignored-attributes");

#ifdef HIPSYCL_TRANSFORM_CLANG_DIR
    std::string clangIncludeDir =
     BOOST_PP_STRINGIZE(HIPSYCL_TRANSFORM_CLANG_DIR);

    if(clangIncludeDir.size() > 0 && clangIncludeDir[clangIncludeDir.size()-1] != '/')
      clangIncludeDir += "/";

     clangIncludeDir +=
        std::string("../lib/clang/")+std::string(CLANG_VERSION_STRING)
        +"/include";

    modifiedArgs.push_back("-I"+clangIncludeDir);
#endif

    return modifiedArgs;
  };
  tool.appendArgumentsAdjuster(adjuster);

  using FrontendActionType = hipsycl::transform::HipsyclTransfromFrontendAction;
  return tool.run(newFrontendActionFactory<FrontendActionType>().get());
}