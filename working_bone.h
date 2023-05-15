#define WORKING_BONE_H

#include "core/math/math_funcs.h"
#include "core/templates/vector.h"
#include "ik_node_3d.h"
#include <limits>

class ConstraintState;
class Constraint;
class TargetState;
class ArmatureSegment;

class WorkingBone
{
public:
    WorkingBone(Ref<IKNode3D> to_simulate, ArmatureSegment chain);

    void set_as_segment_root();
    void fast_update_optimal_rotation_to_pinned_descendants(real_t dampening, bool translate);
    void update_optimal_rotation_to_pinned_descendants(real_t dampening, bool translate,
                                                       Vector<Vector3> &localized_tip_headings,
                                                       Vector<Vector3> &localized_target_headings,
                                                       Vector<real_t> &weights);

private:
    Ref<IKNode3D> for_bone;
    ConstraintState cnstrntstate;
    Constraint constraint;
    TargetState target_state;
    Ref<IKNode3D> sim_target_axes;
    Ref<IKNode3D> sim_local_axes;
    Ref<IKNode3D> sim_constraint_swing_axes;
    Ref<IKNode3D> sim_constraint_twist_axes;
    ArmatureSegment on_chain;
    real_t cos_half_dampen;
    real_t cos_half_return_damp;
    real_t return_damp;
    bool springy;
    bool is_segment_root;
    bool has_pinned_ancestor;
};

#endif // WORKING_BONE_H