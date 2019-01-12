#include "annunciator.h"

#include <glibmm/main.h>
#include <glibmm/spawn.h>

#include <cassert>
#include <iostream>
#include <unordered_map>
// #include "util/misc.h"

namespace
{
    const unsigned int MAX_AGE = 20;

    unsigned int next_id = 0;

    std::unordered_map<unsigned int, Annunciator::Message *> &registered()
    {
        static std::unordered_map<unsigned int, Annunciator::Message *> r;
        return r;
    }

    std::vector<Annunciator::Message *> displayed;
}  // namespace

Annunciator::Message::Message(const Glib::ustring &text, TriggerMode mode,
                              Severity severity)
    : mode(mode),
      severity(severity),
      id(next_id++),
      text(text),
      active_(false),
      age_(0),
      displayed_(false)
{
    registered()[id] = this;
}

Annunciator::Message::~Message()
{
    hide();
    registered().erase(id);
}

void Annunciator::Message::active(bool actv)
{
    assert(mode == TriggerMode::LEVEL);

    if (actv != active_)
    {
        active_ = actv;
        for (std::size_t i = 0; i < displayed.size(); ++i)
        {
            if (displayed[i] == this)
            {
                if (actv)
                {
                    signal_message_reactivated.emit(i);
                }
                else
                {
                    signal_message_deactivated.emit(i);
                }
            }
        }
        one_second_connection.disconnect();
        if (actv)
        {
            age_ = 0;
            if (!displayed_)
            {
                displayed.push_back(this);
                signal_message_activated.emit();
                displayed_ = true;
            }
            std::cout << "[ANNUNCIATOR] " << text << '\n';
        }
        else
        {
            one_second_connection = Glib::signal_timeout().connect_seconds(
                sigc::mem_fun(this, &Annunciator::Message::on_one_second), 1);
        }
    }
}

void Annunciator::Message::fire()
{
    assert(mode == TriggerMode::EDGE);

    age_ = 0;
    if (!one_second_connection.connected())
    {
        one_second_connection = Glib::signal_timeout().connect_seconds(
            sigc::mem_fun(this, &Annunciator::Message::on_one_second), 1);
    }

    std::cout << "[ANNUNCIATOR] " << text << '\n';

    for (std::size_t i = 0; i < displayed.size(); ++i)
    {
        if (displayed[i] == this)
        {
            signal_message_reactivated.emit(i);
            signal_message_deactivated.emit(i);
            return;
        }
    }

    if (!displayed_)
    {
        displayed.push_back(this);
        signal_message_activated.emit();
        signal_message_deactivated.emit(displayed.size() - 1);
        displayed_ = true;
    }
}

bool Annunciator::Message::on_one_second()
{
    if (age_ < MAX_AGE)
    {
        ++age_;
        for (std::size_t i = 0; i < displayed.size(); ++i)
        {
            if (displayed[i] == this)
            {
                signal_message_aging.emit(i);
            }
        }
        return true;
    }
    else
    {
        hide();
        return false;
    }
}

void Annunciator::Message::hide()
{
    for (std::size_t i = 0; i < displayed.size(); ++i)
    {
        if (displayed[i] == this)
        {
            signal_message_hidden.emit(i);
            displayed.erase(
                displayed.begin() +
                static_cast<std::vector<Annunciator::Message *>::difference_type>(i));
            --i;
        }
    }
    displayed_ = false;
}

void Annunciator::Message::set_text(const Glib::ustring &t)
{
    text = t;
    for (std::size_t i = 0; i < displayed.size(); ++i)
    {
        if (displayed[i] == this)
        {
            signal_message_text_changed.emit(i);
        }
    }
}

const std::vector<Annunciator::Message *> &Annunciator::visible()
{
    return displayed;
}

sigc::signal<void> Annunciator::signal_message_activated;

sigc::signal<void, std::size_t> Annunciator::signal_message_deactivated;

sigc::signal<void, std::size_t> Annunciator::signal_message_text_changed;

sigc::signal<void, std::size_t> Annunciator::signal_message_aging;

sigc::signal<void, std::size_t> Annunciator::signal_message_reactivated;

sigc::signal<void, std::size_t> Annunciator::signal_message_hidden;
