#ifndef UTIL_ANNUNCIATOR_H
#define UTIL_ANNUNCIATOR_H

#include <glibmm/ustring.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <cstddef>
#include <vector>
#include "util/noncopyable.h"

namespace Annunciator
{
/**
 * \brief A message that can be displayed in an annunciator panel.
 *
 * It is expected that an object that wishes to announce messages will contain
 * one or more instances of this class as members, one for each message, and
 * will call \ref active to turn them on and off.
 */
class Message final : public NonCopyable, public sigc::trackable
{
   public:
    /**
     * \brief The possible triggering modes for a message.
     */
    enum class TriggerMode
    {
        /**
         * \brief Indicates that the message is either asserted or deasserted at
         * any given time, and is put into those states by the caller.
         */
        LEVEL,

        /**
         * \brief Indicates that the message reflects an event which can occur
         * at some instant in time.
         */
        EDGE,
    };

    /**
     * \brief The possible severity levels of a message.
     */
    enum class Severity
    {
        /**
         * \brief Indicates that the message is not very severe.
         */
        LOW,

        /**
         * \brief Indicates that the message is severe.
         */
        HIGH,
    };

    /**
     * \brief Registers a new message source with the annunciator system.
     *
     * \param[in] text the text of the message
     *
     * \param[in] mode the trigger mode of the message
     *
     * \param[in] severity the severity of the message
     */
    explicit Message(
        const Glib::ustring &text, TriggerMode mode, Severity severity);

    /**
     * \brief Unregisters the message source.
     */
    ~Message();

    /**
     * \brief The trigger mode of the message.
     */
    const TriggerMode mode;

    /**
     * \brief The severity of the message.
     */
    const Severity severity;

    /**
     * \brief A globally-unique ID number that refers to this message.
     */
    const unsigned int id;

    /**
     * \brief Checks whether the message is active.
     *
     * \return \c true if the message is active, or \c false if not (always \c
     * false if \c mode is TRIGGER_EDGE)
     */
    bool active() const;

    /**
     * \brief Sets whether a level-triggered message is active or not.
     *
     * \pre \ref mode must be TRIGGER_LEVEL.
     *
     * \param[in] actv \c true to activate the message, or \c false to
     * deactivate it
     */
    void active(bool actv);

    /**
     * \brief Fires an edge-triggered message.
     *
     * \pre \c mode must be TRIGGER_EDGE.
     */
    void fire();

    /**
     * \brief Returns the age of the message.
     *
     * \return the age of the message, in seconds (that is, the amount of time
     * since the message was last active)
     */
    unsigned int age() const;

    /**
     * \brief Returns the text of the message.
     *
     * \return the text of the message
     */
    const Glib::ustring &get_text() const;

    /**
     * \brief Sets the text of the message.
     *
     * \param[in] text the new text of the message
     */
    void set_text(const Glib::ustring &text);

   private:
    Glib::ustring text;
    bool active_;
    unsigned int age_;
    bool displayed_;
    sigc::connection one_second_connection;

    bool on_one_second();
    void hide();
};

/**
 * \brief Returns the currently visible (active and aging) messages.
 *
 * \return the messages
 */
const std::vector<Message *> &visible();

/**
 * \brief Fired when a hidden message is activated.
 */
extern sigc::signal<void> signal_message_activated;

/**
 * \brief Fired when an active message is deactivated.
 */
extern sigc::signal<void, std::size_t> signal_message_deactivated;

/**
 * \brief Fired when a visible message’s text changes.
 */
extern sigc::signal<void, std::size_t> signal_message_text_changed;

/**
 * \brief Fired when a message ages.
 */
extern sigc::signal<void, std::size_t> signal_message_aging;

/**
 * \brief Fired when a still-visible but deactivated message is reactivated.
 */
extern sigc::signal<void, std::size_t> signal_message_reactivated;

/**
 * \brief Fired when a message is hidden.
 */
extern sigc::signal<void, std::size_t> signal_message_hidden;
}

inline bool Annunciator::Message::active() const
{
    return active_;
}

inline unsigned int Annunciator::Message::age() const
{
    return age_;
}

inline const Glib::ustring &Annunciator::Message::get_text() const
{
    return text;
}

#endif
