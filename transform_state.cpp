#include "transform_state.h"

TransformState::TransformState(SkeletonState &skeletonState, std::string id, std::vector<double> translation, std::vector<double> rotation, std::vector<double> scale, std::string parent_id, void* directReference)
    : skeletonState(skeletonState), id(id), translation(translation), rotation(rotation), scale(scale), parent_id(parent_id), directReference(directReference) {}

void TransformState::update(std::vector<double> translation, std::vector<double> rotation, std::vector<double> scale) {
    this->translation = translation;
    this->rotation = rotation;
    this->scale = scale;
}

void TransformState::prune() {
    // Implementation for prune method
    skeletonState.transformsList[this->index] = nullptr;
    skeletonState.transformMap.erase(this->id);
}

int TransformState::getIndex() {
    return this->index;
}

int TransformState::getParentIndex() {
    return this->parentIdx;
}

TransformState* TransformState::getParentTransform() {
    // Implementation for getParentTransform method
    return skeletonState.transforms[this->parentIdx];
}

void TransformState::setIndex(int index) {
    this->index = index;
    TransformState* parTransform = skeletonState.transformMap[this->parent_id];
    if (parTransform != nullptr) {
        parTransform->addChild(this->index);
    }
}

void TransformState::addChild(int childIndex) {
    this->childIdxsList.push_back(childIndex);
}

void TransformState::optimize() {
    this->childIndices.resize(this->childIdxsList.size());
    for (size_t i = 0; i < this->childIdxsList.size(); i++) {
        this->childIndices[i] = this->childIdxsList[i];
    }

    if (this->parent_id != "") {
        this->parentIdx = skeletonState.transformMap[this->parent_id]->getIndex();
    }
}

void TransformState::validate() {
    if (skeletonState.assumeValid) {
        return;
    }

    if (this->parent_id.empty()) {
        return;
    }

    TransformState* parent = skeletonState.transformMap[this->parent_id];
    if (parent == nullptr) {
        throw std::runtime_error("Transform '" + this->id + "' lists '" + this->parent_id + "' as its parent, but no transform with id '" + this->parent_id + "' has been registered with this SkeletonState");
    }

    TransformState* ancestor = parent;
    while (ancestor != nullptr) {
        if (ancestor == this) {
            throw std::runtime_error("Transform '" + this->id + "' is listed as being both a descendant and an ancestor of itself.");
        }

        if (ancestor->parent_id.empty()) {
            ancestor = nullptr;
        } else {
            TransformState* curr = ancestor;
            ancestor = skeletonState.transformMap[ancestor->parent_id];
            if (ancestor == nullptr) {
                throw std::runtime_error("Transform with id `" + curr->id + "` lists its parent transform as having id `" + curr->parent_id + "`, but no such transform has been registered with the SkeletonState");
            }
        }
    }
}


std::string TransformState::getIdString() {
    return this->id;
}
