/*
 * Copyright (c) 2013 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/Base/foreach.h>

#include <Eve-Xin/Controllers/Skill.h>

namespace EveXin {
	class Skill;
	class SkillItem : public boost::enable_shared_from_this<SkillItem> {
		public:
			typedef boost::shared_ptr<SkillItem> ref;
			SkillItem(boost::shared_ptr<SkillItem> parent, boost::shared_ptr<Skill> skill) : parent_(parent), skill_(skill) {
			}

			SkillItem(boost::shared_ptr<SkillItem> parent, const std::string& id, const std::string& name) : parent_(parent), id_(id), name_(name) {
			}

			virtual ~SkillItem() {}

			virtual void addChild(ref child) {
				children_[child->getID()] = child;
			}

			virtual std::vector<SkillItem::ref> getChildren() const {
				std::vector<SkillItem::ref> result;
				foreach (auto child, children_) {
					result.push_back(child.second);
				}
				std::sort(result.begin(), result.end(), [](SkillItem::ref a, SkillItem::ref b){return a->getName() < b->getName();});
				return result;
			}

			SkillItem::ref getChild(const std::string& id) {
				return hasChild(id) ? children_[id] : SkillItem::ref();
			}

			/**
			 * Get a child with the matching id, and if none exists create such a group
			 * with the given name.
			 */
			SkillItem::ref getGroup(const std::string& id, const std::string& name) {
				if (hasChild(id)) {
					return getChild(id);
				}
				SkillItem::ref group = boost::make_shared<SkillItem>(shared_from_this(), id, name);
				addChild(group);
				return group;
			}

			bool hasChild(const std::string& id) {
				return children_.find(id) != children_.end();
			}

			virtual std::string getID() const {
				return skill_ ? skill_->getID() : id_;
			}

			virtual std::string getName() const {
				return skill_ ? skill_->getName() : name_;
			}

			boost::shared_ptr<Skill> getSkill() const {return skill_;}

			/** Recurse down until finding a skill with the matching ID */
			boost::shared_ptr<SkillItem> findSkillItem(const std::string& id) {
				if (skill_) {
					if (id_ == id) {
						return shared_from_this();
					}
				}
				else {
					if (hasChild(id)) { // n.b. this may not be a skill
						SkillItem::ref item = getChild(id);
						if (item->getSkill()) {
							return item;
						}
					}
					else {
						foreach (auto child, children_) {
							SkillItem::ref item = child.second->findSkillItem(id);
							if (item) {
								return item;
							}
						}
					}
				}
				return ref();
			}

			/**
			 * Get the parent in whatever skill tree this is.
			 * 
			 * This is not the group (or may not be the group) in which it lives.
			 */
			boost::shared_ptr<SkillItem> getParent() const {
				return parent_.lock();
			}

			bool operator==(const SkillItem& other) {
				bool equal = true;//parent_ ? parent_.lock().get() == other.parent_.lock().get() : !other.parent_;
				auto myChildren = getChildren();
				auto otherChildren = other.getChildren();
				equal &= myChildren.size() == otherChildren.size();
				for (size_t i = 0; equal && i < myChildren.size(); i++) {
					equal &= *myChildren[i] == *otherChildren[i];
				}
				equal &= id_ == other.id_ && name_ == other.name_;
				//std::cerr << "Skill equality " << equal << std::endl;
				return equal;
			}

			bool operator!=(const SkillItem& other) {
				return !(*this == other);
			}

		public:
			boost::signal<void()> onChildrenAboutToChange;
			boost::signal<void()> onChildrenChanged;

		protected:
			std::map<std::string/*id*/, ref> children_;			
		private:
			boost::weak_ptr<SkillItem> parent_;
			boost::shared_ptr<Skill> skill_;
			std::string id_;
			std::string name_;
	};
}
