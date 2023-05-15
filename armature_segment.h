#ifndef ARMATURE_SEGMENT_H
#define ARMATURE_SEGMENT_H

#include <core/reference.h>
#include <core/vector.h>
#include "math_utils.h"
#include "shadow_skeleton.h"
#include "bone_state.h"
#include "working_bone.h"
#include "target_state.h"

class ArmatureSegment : public Reference {
    GDCLASS(ArmatureSegment, Reference);

protected:
    static void _bind_methods();

public:
    ArmatureSegment();
    ArmatureSegment(ShadowSkeleton *shadow_skel, BoneState *starting_from, Ref<ArmatureSegment> parent_segment, bool is_root_pinned);

    void build_segment(BoneState *starting_from);
    void build_reverse_traversal_array();
    void create_heading_arrays();
    void recursively_create_penalty_array(Vector<Vector<double>> &weight_array, Vector<WorkingBone *> &pin_sequence, double current_falloff);
    Vector<Ref<ArmatureSegment>> get_descendant_segments();
    double get_manual_MSD(const Vector<Vector3<double>> &loc_tips, const Vector<Vector3<double>> &loc_targets, const Vector<double> &weights);

    Ref<ArmatureSegment> parentSegment;
    bool isRootPinned;
    bool hasPinnedAncestor;
    Vector<Ref<ArmatureSegment>> subSegments;
    Vector<Ref<ArmatureSegment>> childSegments;
    Vector<WorkingBone *> solvableStrandBones;
    Vector<WorkingBone *> reversedTraversalArray;
    Vector<Vector3<double>> boneCenteredTargetHeadings;
    Vector<Vector3<double>> boneCenteredTipHeadings;
    Vector<Vector3<double>> uniform_boneCenteredTipHeadings;
    Vector<double> weights;

    WorkingBone *wb_segmentRoot;
    WorkingBone *wb_segmentTip;

    ShadowSkeleton *shadowSkel;
    Vector<Transform> simTransforms;
    Vector<Transform> forArmature;
};

#endif // ARMATURE_SEGMENT_H
