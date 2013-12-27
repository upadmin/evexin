/*
 * Copyright (c) 2013 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#pragma once

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace EveXin {
	class Skill;
	class SkillLevel {
		public:
			typedef boost::shared_ptr<SkillLevel> ref;
			SkillLevel(boost::weak_ptr<Skill> skill, int level) : skill_(skill), level_(level) {}
		private:
			boost::weak_ptr<Skill> skill_;
			int level_;
	};
}
