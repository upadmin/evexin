/*
 * Copyright (c) 2014 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include <Eve-Xin/Controllers/SkillPlanList.h>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <Swiften/Base/foreach.h>

#include <Eve-Xin/Controllers/SkillPlan.h>

namespace EveXin {
	
SkillPlanList::SkillPlanList(const std::string id, const std::string& name, boost::shared_ptr<SkillTree> allSkills) : SkillItem(SkillItem::ref(), id, name), allSkills_(allSkills), nextID_(0), undoing_(false), renaming_(false) {
	
}


SkillPlanList::~SkillPlanList() {
	std::vector<SkillItem::ref> children = getChildren();
	foreach(SkillItem::ref child, children) {
		SkillPlan::ref plan = boost::dynamic_pointer_cast<SkillPlan>(child);
		if (plan) {
			deletePlan(plan);
		}
	}
}

void SkillPlanList::undo() {
	if (undoActions_.empty()) {
		return;
	}
	bool originalState = undoing_;
	undoing_ = true;
	std::pair<UndoAction, SkillPlan::ref> action = undoActions_.back();
	undoActions_.pop_back();
	switch (action.first) {
		case ModifyPlan: action.second->undo(); break;
		case CreatePlan: deletePlan(action.second); break;
		case DeletePlan: {
			addChild(action.second);
			onWantsToSave(action.second);
			action.second->onWantsToSave.connect(boost::bind(&SkillPlanList::handleSkillPlanWantsToSave, this, action.second));
			onAvailablePlansChanged();
			break;
		}
		case RenamePlan: {
			/* Edit plan itself is empty, and is then a delete and add */
			undo();
			undo();
		}
	}
	undoing_ = originalState;
}

void SkillPlanList::addUndoAction(const std::pair<UndoAction, SkillPlan::ref>& action, bool userAction) {
	if (userAction && !undoing_) {
		undoActions_.push_back(action);
	}
}

SkillPlan::ref SkillPlanList::createPlan(const std::string& name, bool userAction) {
	SkillPlan::ref plan = boost::make_shared<SkillPlan>(shared_from_this(), getID() + "_plan_" + boost::lexical_cast<std::string>(nextID_++), name, allSkills_);
	addUndoAction(std::pair<UndoAction, SkillPlan::ref>(CreatePlan, plan), userAction);
	if (knownSkills_) {
		plan->setKnownSkills(knownSkills_);
	}
	plan->onWantsToSave.connect(boost::bind(&SkillPlanList::handleSkillPlanWantsToSave, this, plan));
	addChild(plan);
	onWantsToSave(plan);
	onAvailablePlansChanged();
	return plan;
}

void SkillPlanList::deletePlan(SkillPlan::ref plan, bool userAction) {
	addUndoAction(std::pair<UndoAction, SkillPlan::ref>(DeletePlan, plan), userAction);
	plan->onWantsToSave.disconnect(boost::bind(&SkillPlanList::handleSkillPlanWantsToSave, this, plan));
	children_.erase(plan->getID());
	onWantsToSave(SkillPlan::ref());
	onAvailablePlansChanged();
}

void SkillPlanList::handleSkillPlanWantsToSave(SkillPlan::ref plan) {
	if (renaming_) {
		return;
	}
	onWantsToSave(plan);
	addUndoAction(std::pair<UndoAction, SkillPlan::ref>(ModifyPlan, plan), true);
}

void SkillPlanList::setKnownSkills(boost::shared_ptr<SkillItem> knownSkills) {
	knownSkills_ = knownSkills;
	std::vector<SkillItem::ref> children = getChildren();
	foreach (auto item, children) {
		auto plan = boost::dynamic_pointer_cast<SkillPlan>(item);
		plan->setKnownSkills(knownSkills_);
	}
}

bool SkillPlanList::renamePlan(SkillPlan::ref plan, const std::string& name) {
	SkillPlan::ref newPlan = createPlan(name, true);
	renaming_ = true;
	auto oldChildren = plan->getChildren();
	foreach (SkillItem::ref item, oldChildren) {
		SkillLevel::ref level = boost::dynamic_pointer_cast<SkillLevel>(item);
		newPlan->addSkill(level->getSkill(), level->getLevel());
	}
	renaming_ = false;
	deletePlan(plan, true);
	addUndoAction(std::pair<UndoAction, SkillPlan::ref>(RenamePlan, SkillPlan::ref()), true);
	onWantsToSave(plan);
	onWantsToSave(newPlan);
	return true; // FIXME start to check if the rename is legal first.
}

}
