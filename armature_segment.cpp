#include "armature_segment.h"

ArmatureSegment::ArmatureSegment(ShadowSkeleton *shadowSkel, BoneState *startingFrom, ArmatureSegment *parentSegment, bool isRootPinned)
    : shadowSkel(shadowSkel),
      simTransforms(shadowSkel->simTransforms),
      forArmature(shadowSkel->forArmature),
      parentSegment(parentSegment),
      isRootPinned(isRootPinned),
      hasPinnedAncestor(parentSegment != nullptr && (parentSegment->isRootPinned || parentSegment->hasPinnedAncestor)),
      qcpConverger(MathUtils::DOUBLE_ROUNDING_ERROR, MathUtils::DOUBLE_ROUNDING_ERROR)
{
    build_segment(startingFrom);
    if (this->isRootPinned)
        this->wb_segmentRoot->set_as_segment_root();
    build_reverse_traversal_array();
    create_heading_arrays();
}

double get_dampening() {
    return shadowSkel->base_dampening;
}

void ArmatureSegment::build_segment(BoneState *startingFrom)
{
    Vector<WorkingBone *> seg_effectors;
    Vector<WorkingBone *> strand_bones;
    Vector<Ref<ArmatureSegment>> sub_sgmts;
    Vector<Ref<ArmatureSegment>> child_sgmts;
    BoneState *current_bs = startingFrom;
    bool finished = false;
    while (!finished)
    {
        Ref<WorkingBone> current_wb = memnew(WorkingBone(current_bs, this));
        if (current_bs == startingFrom)
            this->wb_segmentRoot = current_wb;
        strand_bones.push_back(current_wb);
        TargetState *target = current_bs->get_target();
        if (target != nullptr || current_bs->get_child_count() > 1)
        {
            if (target != nullptr)
            {
                seg_effectors.push_back(current_wb);
                if (target->get_depth_fall_off() <= 0.0)
                {
                    this->wb_segmentTip = current_wb;
                    finished = true;
                }
            }
            if (finished)
            {
                for (int i = 0; i < current_bs->get_child_count(); i++)
                    child_sgmts.push_back(memnew(ArmatureSegment(shadowSkel, current_bs->get_child(i), this, true)));
            }
            else
            {
                for (int i = 0; i < current_bs->get_child_count(); i++)
                {
                    Ref<ArmatureSegment> subseg = memnew(ArmatureSegment(shadowSkel, current_bs->get_child(i), this, false));
                    sub_sgmts.push_back(subseg);
                    sub_sgmts.append_array(subseg->subSegments);
                    seg_effectors.append_array(subseg->pinnedBones);
                }
                finished = true;
                this->wb_segmentTip = current_wb;
            }
        }
        else if (current_bs->get_child_count() == 1)
        {
            current_bs = current_bs->get_child(0);
        }
        else
        {
            this->wb_segmentTip = current_wb;
        }
    }
    this->subSegments = sub_sgmts;
    this->pinnedBones = seg_effectors;
    this->childSegments = child_sgmts;
    this->solvableStrandBones = strand_bones;
}

void ArmatureSegment::build_reverse_traversal_array()
{
    Vector<WorkingBone *> reverse_traversal_array;
    for (WorkingBone *wb : solvableStrandBones)
    {
        if (wb->forBone->get_stiffness() < 1.0)
        {
            reverse_traversal_array.push_back(wb);
        }
    }
    for (const Ref<ArmatureSegment> &ss : subSegments)
    {
        reverse_traversal_array.append_array(ss->reversedTraversalArray);
    }
    this->reversedTraversalArray = reverse_traversal_array;
}

void ArmatureSegment::create_heading_arrays()
{
    Vector<Vector<double>> penalty_array;
    Vector<WorkingBone *> pin_sequence;
    recursively_create_penalty_array(penalty_array, pin_sequence, 1.0);
    int total_headings = 0;
    for (const Vector<double> &a : penalty_array)
    {
        total_headings += a.size();
    }
    boneCenteredTargetHeadings.resize(total_headings);
    boneCenteredTipHeadings.resize(total_headings);
    uniform_boneCenteredTipHeadings.resize(total_headings);
    weights.resize(total_headings);
    int current_heading = 0;
    for (const Vector<double> &a : penalty_array)
    {
        for (double ad : a)
        {
            weights[current_heading] = ad;
            boneCenteredTargetHeadings[current_heading] = Vector3<double>();
            boneCenteredTipHeadings[current_heading] = Vector3<double>();
            uniform_boneCenteredTipHeadings[current_heading] = Vector3<double>();
            current_heading++;
        }
    }
}

void ArmatureSegment::recursively_create_penalty_array(Vector<Vector<double>> &weight_array, Vector<WorkingBone *> &pin_sequence, double current_falloff)
{
    if (current_falloff == 0)
    {
        return;
    }
    else
    {
        TargetState *target = this->wb_segmentTip->targetState;
        if (target != nullptr)
        {
            Vector<double> inner_weight_array;
            weight_array.push_back(inner_weight_array);
            byte mode_code = target->get_mode_code();
            inner_weight_array.push_back(target->get_weight() * current_falloff);
            double max_pin_weight = target->get_max_priority();

            if (max_pin_weight == 0)
                max_pin_weight = 1;

            if ((mode_code & TargetState::XDir) != 0)
            {
                double sub_target_weight = target->get_weight() * (target->get_priority(TargetState::XDir) / max_pin_weight) * current_falloff;
                inner_weight_array.push_back(sub_target_weight);
                inner_weight_array.push_back(sub_target_weight);
            }
            if ((mode_code & TargetState::YDir) != 0)
            {
                double sub_target_weight = target->get_weight() * (target->get_priority(TargetState::YDir) / max_pin_weight) * current_falloff;
                inner_weight_array.push_back(sub_target_weight);
                inner_weight_array.push_back(sub_target_weight);
            }
            if ((mode_code & TargetState::ZDir) != 0)
            {
                double sub_target_weight = target->get_weight() * (target->get_priority(TargetState::ZDir) / max_pin_weight) * current_falloff;
                inner_weight_array.push_back(sub_target_weight);
                inner_weight_array.push_back(sub_target_weight);
            }
            pin_sequence.push_back(wb_segmentTip);
        }

        double this_falloff = target == nullptr ? 1 : target->get_depth_fall_off();
        for (const Ref<ArmatureSegment> &s : subSegments)
        {
            s->recursively_create_penalty_array(weight_array, pin_sequence, current_falloff * this_falloff);
        }
    }
}

Vector<Ref<ArmatureSegment>> ArmatureSegment::get_descendant_segments()
{
    Vector<Ref<ArmatureSegment>> result;
    result.push_back(Ref<ArmatureSegment>(this));
    for (const Ref<ArmatureSegment> &child : childSegments)
    {
        Vector<Ref<ArmatureSegment>> child_descendants = child->get_descendant_segments();
        result.append_array(child_descendants);
    }
    return result;
}

double ArmatureSegment::get_manual_MSD(const Vector<Vector3<double>> &loc_tips, const Vector<Vector3<double>> &loc_targets, const Vector<double> &weights)
{
    double manual_RMSD = 0.0;
    double wsum = 0.0;
    for (int i = 0; i < loc_targets.size(); i++)
    {
        double xd = loc_targets[i].x - loc_tips[i].x;
        double yd = loc_targets[i].y - loc_tips[i].y;
        double zd = loc_targets[i].z - loc_tips[i].z;
        double magsq = weights[i] * (xd * xd + yd * yd + zd * zd);
        manual_RMSD += magsq;
        wsum += weights[i];
    }
    manual_RMSD /= wsum * wsum;
    return manual_RMSD;
}