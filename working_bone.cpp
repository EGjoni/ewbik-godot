#include "core/math/math_funcs.h"
#include "core/templates/vector.h"
#include "ik_node_3d.h"
#include <limits>

class WorkingBone
{
public:
    Ref<IKNode3D> for_bone;
    ConstraintState cnstrntstate;
    Constraint constraint;
    TargetState target_state;
    Ref<IKNode3D> sim_target_axes;
    Ref<IKNode3D> sim_local_axes;
    Ref<IKNode3D> sim_constraint_swing_axes;
    Ref<IKNode3D> sim_constraint_twist_axes;
    ArmatureSegment on_chain;
    real_t cos_half_dampen = 0.0;
    real_t cos_half_return_damp = 0.0;
    real_t return_damp = 0.0;
    bool springy = false;
    bool is_segment_root = false;
    bool has_pinned_ancestor = false;

    WorkingBone(Ref<IKNode3D> to_simulate, ArmatureSegment chain)
        : for_bone(to_simulate), on_chain(chain)
    {
        cnstrntstate = for_bone->get_constraint();
        sim_local_axes = sim_transforms[for_bone->get_transform().get_index()];
        if (for_bone->get_target() != nullptr)
        {
            this->target_state = for_bone->get_target();
            this->sim_target_axes = sim_transforms[target_state.get_transform().get_index()];
        }
        this->has_pinned_ancestor = on_chain.has_pinned_ancestor;
        real_t predamp = 1.0 - for_bone->get_stiffness();
        real_t default_dampening = on_chain.get_dampening();
        real_t dampening = for_bone->get_parent() == nullptr ? Math_PI : predamp * default_dampening;
        cos_half_dampen = Math::cos(dampening / 2.0);
        if (cnstrntstate != nullptr)
        {
            constraint = cnstrntstate.get_direct_reference();
            sim_constraint_swing_axes = sim_transforms[cnstrntstate.get_swing_transform().get_index()];
            sim_constraint_twist_axes = cnstrntstate.get_twist_transform() == nullptr
                                            ? nullptr
                                            : sim_transforms[cnstrntstate.get_twist_transform().get_index()];
            Ref<AbstractKusudama> k = dynamic_cast<AbstractKusudama *>(cnstrntstate.get_direct_reference());
            if (k.is_valid() && k->get_painfulness() > 0.0)
            {
                springy = true;
            }
            else
            {
                springy = false;
            }
        }
    }

    void setAsSegmentRoot()
    {
        this->isSegmentRoot = true;
    }

    void fastUpdateOptimalRotationToPinnedDescendants(int stabilizationPasses, bool translate)
    {
        simLocalAxes->_propagate_transform_changed();
        real_t newDampening = -1;
        if (translate)
        {
            newDampening = Math_PI * 2;
        }
        updateTargetHeadings(onChain.boneCenteredTargetHeadings, onChain.weights);
        Quat prevOrientation = simLocalAxes->get_transform().basis.get_rotation_quat();
        bool gotCloser = true;
        for (int i = 0; i <= stabilizationPasses; i++)
        {
            updateTipHeadings(onChain.boneCenteredTipHeadings, true);
            updateOptimalRotationToPinnedDescendants(newDampening, translate, onChain.boneCenteredTipHeadings,
                                                     onChain.boneCenteredTargetHeadings, weights);
            if (stabilizationPasses > 0)
            {
                updateTipHeadings(onChain.uniform_boneCenteredTipHeadings, false);
                real_t currentmsd = onChain.getManualMSD(onChain.uniform_boneCenteredTipHeadings,
                                                         onChain.boneCenteredTargetHeadings, onChain.weights);
                if (currentmsd <= onChain.previousDeviation * 1.000001)
                {
                    onChain.previousDeviation = currentmsd;
                    gotCloser = true;
                    break;
                }
                else
                {
                    gotCloser = false;
                }
            }
        }
        if (!gotCloser)
        {
            simLocalAxes->set_transform(simLocalAxes->get_transform().scaled(simLocalAxes->get_scale()).rotated(prevOrientation));
        }

        if (onChain.wb_segmentRoot == this)
        {
            onChain.previousDeviation = Math_INF;
        }
        simLocalAxes->_propagate_transform_changed();
    }
    void updateOptimalRotationToPinnedDescendants(real_t dampening, bool translate,
                                                  Vector<Vector3> &localizedTipHeadings,
                                                  Vector<Vector3> &localizedTargetHeadings,
                                                  Vector<real_t> &weights)
    {
        Quat qcpRot = onChain.qcpConverger.weightedSuperpose(localizedTipHeadings, localizedTargetHeadings, weights,
                                                             translate);

        Vector3 translateBy = onChain.qcpConverger.getTranslation();
        real_t boneDamp = cosHalfDampen;
        if (dampening != -1)
        {
            boneDamp = dampening;
            qcpRot.clamp_to_angle(boneDamp);
        }
        else
        {
            qcpRot.clamp_to_quadrance_angle(boneDamp);
        }
        simLocalAxes.rotate_by(qcpRot);
        if (translate)
        {
            simLocalAxes.translate_by_global(translateBy);
        }
        simLocalAxes.update_global();

        if (constraint != nullptr)
        {
            constraint.set_axes_to_snapped(simLocalAxes, simConstraintSwingAxes, simConstraintTwistAxes);
            if (translate)
            {
                simConstraintSwingAxes.translate_by_global(translateBy);
                simConstraintTwistAxes.translate_by_global(translateBy);
            }
        }
    }
}
