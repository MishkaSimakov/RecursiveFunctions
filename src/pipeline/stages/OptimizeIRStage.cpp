#include "OptimizeIRStage.h"

#include "passes/PassManager.h"
#include "passes/branch_to_select/ReplaceBranchWithSelect.h"
#include "passes/common_elimination/CommonEliminationPass.h"
#include "passes/constant_propagation/ConstantPropagationPass.h"
#include "passes/inline/InlinePass.h"
#include "passes/loop_rotation/LoopRotationPass.h"
#include "passes/phi_elimination/PhiEliminationPass.h"
#include "passes/print/PrintPass.h"
#include "passes/recursion_to_loop/RecursionToLoopPass.h"
#include "passes/registers_allocation/RegisterAllocationPass.h"
#include "passes/silly_move_erasure/SSAMoveErasure.h"
#include "passes/silly_move_erasure/SillyMoveErasurePass.h"
#include "passes/unused_elimination/UnusedFunctionsEliminationPass.h"
#include "passes/unused_elimination/UnusedTemporariesEliminationPass.h"

IR::Program OptimizeIRStage::apply(IR::Program program) {
  Passes::PassManager pass_manager;

  pass_manager.register_pass<Passes::UnusedFunctionsEliminationPass>();
  pass_manager.register_pass<Passes::UnusedTemporariesEliminationPass>();

  pass_manager.register_pass<Passes::CommonEliminationPass>();

  pass_manager.register_pass<Passes::RecursionToLoopPass>();

  pass_manager.register_pass<Passes::LoopRotationPass>();
  pass_manager.register_pass<Passes::ConstantPropagationPass>();

  pass_manager.register_pass<Passes::InlinePass>();

  pass_manager.register_pass<Passes::SSAMoveErasure>();
  pass_manager.register_pass<Passes::ConstantPropagationPass>();
  pass_manager.register_pass<Passes::SSAMoveErasure>();

  pass_manager.register_pass<Passes::ReplaceBranchWithSelect>();

  pass_manager.register_pass<Passes::UnusedFunctionsEliminationPass>();
  pass_manager.register_pass<Passes::UnusedTemporariesEliminationPass>();

  pass_manager.register_pass<Passes::ConstantPropagationPass>();

  pass_manager.register_pass<Passes::PhiEliminationPass>();

  pass_manager.register_pass<Passes::RegisterAllocationPass>();

  pass_manager.register_pass<Passes::SillyMoveErasurePass>();

  pass_manager.apply(program);

  return std::move(program);
}
