#include "trigger_limit.hpp"
#include "trigger.hpp"
#include <algorithm>

std::list<std::shared_ptr<lob::trigger>>::iterator lob::trigger_limit::insert(
    const std::shared_ptr<trigger> &t_trigger) {
	m_triggers.push_back(t_trigger);
	return std::prev(m_triggers.end());
}


void lob::trigger_limit::erase(
    const std::list<std::shared_ptr<trigger>>::iterator &t_trigger_it) {
	auto trigger_obj = *t_trigger_it;
	trigger_obj->m_queued = false;
	m_triggers.erase(t_trigger_it);
}

void lob::trigger_limit::trigger_all() {

	while (!m_triggers.empty()) {
		auto trigger_obj = m_triggers.front();
		m_triggers.pop_front();
		trigger_obj->m_queued = false;
		trigger_obj->on_triggered();

		if (!trigger_obj
			 ->m_queued) { // on_triggered may reinsert the trigger
			trigger_obj->m_book = nullptr;
		}
	}
}

lob::trigger_limit::~trigger_limit() {
	for (auto &trigger : m_triggers) {
		trigger->m_book = nullptr;
		trigger->m_queued = false;
	}
}