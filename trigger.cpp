#include "trigger.hpp"
#include "trigger_limit.hpp"
#include "book.hpp"

lob::trigger::trigger(lob::side t_side, double t_price): m_side(t_side), m_price(t_price) {}

bool lob::trigger::cancel() {
    if(m_queued) {
        
        m_limit_it->second.erase(m_trigger_it);

        if(m_limit_it->second.is_empty()) {
            if(m_side == side::bid) {
                m_book->m_bid_triggers.erase(m_limit_it);
            } else {
                m_book->m_ask_triggers.erase(m_limit_it); 
            }
        }

        on_canceled();
        
        if(!m_queued) { // on_canceled may reinsert the trigger
            m_book = nullptr;
        }

        return true;
    }

    return false;
}

void lob::trigger::set_price(double t_price) {
    if(m_price == t_price) {
        return;
    }

    if(m_queued) {
        m_limit_it->second.erase(m_trigger_it);

        if(m_limit_it->second.is_empty()) {
            if(m_side == side::bid) {
                m_book->m_bid_triggers.erase(m_limit_it);
            } else {
                m_book->m_ask_triggers.erase(m_limit_it); 
            }
        }
    }

    m_price = t_price;
    m_book->insert(shared_from_this());
}