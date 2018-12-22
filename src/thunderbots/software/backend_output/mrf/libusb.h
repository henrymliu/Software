#ifndef UTIL_LIBUSB_H
#define UTIL_LIBUSB_H

#include <glib.h>
#include <libusb.h>
#include <sigc++/connection.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "async_operation.h"
#include "util/noncopyable.h"

namespace USB
{
/**
 * \cond
 * These are for internal use only.
 */
extern "C" {
void usb_context_pollfd_add_trampoline(int fd, short events, void *user_data);
void usb_context_pollfd_remove_trampoline(int fd, void *user_data);
void usb_transfer_handle_completed_transfer_trampoline(
    libusb_transfer *transfer);
}
/**
 * \endcond
 */

/**
 * \brief An error that occurs in a libusb library function.
 */
class Error : public std::runtime_error
{
   public:
    /**
     * \brief Constructs a new error object with a message.
     *
     * \param[in] msg a detail message explaining the error
     */
    explicit Error(const std::string &msg);
};

/**
 * \brief An error that occurs on a USB transfer.
 */
class TransferError : public Error
{
   public:
    /**
     * \brief Constructs a new transfer error.
     *
     * \param[in] endpoint the endpoint number, with bit 7 used to indicate
     * direction
     *
     * \param[in] msg a detail message explaining the error (UTF-8 encoded)
     */
    explicit TransferError(unsigned int endpoint, const char *msg);
};

/**
 * \brief An error that occurs when a USB transfer times out.
 */
class TransferTimeoutError final : public TransferError
{
   public:
    /**
     * \brief Constructs a new timeout error object.
     *
     * \param[in] endpoint the endpoint number, with bit 7 used to indicate
     * direction
     */
    explicit TransferTimeoutError(unsigned int endpoint);
};

/**
 * \brief An error that occurs when a USB stall occurs.
 */
class TransferStallError final : public TransferError
{
   public:
    /**
     * \brief Constructs a new stall error object.
     *
     * \param[in] endpoint the endpoint number, with bit 7 used to indicate
     * direction
     */
    explicit TransferStallError(unsigned int endpoint);
};

/**
 * \brief An error that occurs when a USB transfer is cancelled.
 */
class TransferCancelledError final : public TransferError
{
   public:
    /**
     * \brief Constructs a new cancelled transfer error object.
     *
     * \param[in] endpoint the endpoint number, with bit 7 used to indicate
     * direction
     */
    explicit TransferCancelledError(unsigned int endpoint);
};

/**
 * \brief A libusb context.
 */
class Context final : public NonCopyable
{
   public:
    /**
     * \brief Initializes the library and creates a context.
     */
    explicit Context();

    void handle_usb_fds();

    /**
     * \brief Deinitializes the library and destroys the context.
     *
     * This must be invoked after all associated DeviceList and DeviceHandle
     * objects have been destroyed.
     */
    ~Context();

   private:
    friend class DeviceList;
    friend class DeviceHandle;
    friend void usb_context_pollfd_add_trampoline(
        int fd, short events, void *user_data);
    friend void usb_context_pollfd_remove_trampoline(int fd, void *user_data);

    libusb_context *context;
    std::unordered_map<int, sigc::connection> fd_connections;

    void add_pollfd(int fd, short events);
    void remove_pollfd(int fd);
};

/**
 * \brief A collection of information about a USB device.
 */
class Device final
{
   public:
    /**
     * \brief Makes a copy of a device record.
     *
     * \param[in] copyref the object to copy
     */
    Device(const Device &copyref);

    /**
     * \brief Destroys the device information record.
     */
    ~Device();

    /**
     * \brief Assigns a device information record.
     *
     * \param[in] assgref the object to copy from
     *
     * \return this object
     */
    Device &operator=(const Device &assgref);

    /**
     * \brief Returns the 16-bit vendor ID from the device’s device descriptor.
     *
     * \return the vendor ID
     */
    unsigned int vendor_id() const
    {
        return device_descriptor.idVendor;
    }

    /**
     * \brief Returns the 16-bit product ID from the device’s device descriptor.
     *
     * \return the product ID
     */
    unsigned int product_id() const
    {
        return device_descriptor.idProduct;
    }

    /**
     * \brief Returns the serial number from the device’s device and string
     * descriptors.
     *
     * \return the serial number, or an empty string if the device does not
     * expose a serial number
     */
    std::string serial_number() const;

   private:
    friend class DeviceList;
    friend class DeviceHandle;

    libusb_context *context;
    libusb_device *device;
    libusb_device_descriptor device_descriptor;

    explicit Device(libusb_device *device);
};

/**
 * \brief A list of USB devices.
 */
class DeviceList final : public NonCopyable
{
   public:
    /**
     * \brief Constructs a list of all USB devices attached to the system.
     *
     * \param[in] context the library context in which to operate
     */
    explicit DeviceList(Context &context);

    /**
     * \brief Frees the list of devices.
     */
    ~DeviceList();

    /**
     * \brief Returns the size of the list.
     *
     * \return the number of devices in the list
     */
    std::size_t size() const
    {
        return size_;
    }

    /**
     * \brief Returns a device from the list.
     *
     * \param[in] i the index of the device to return, counting from zero
     *
     * \return the device
     */
    Device operator[](const std::size_t i) const;

   private:
    std::size_t size_;
    libusb_device **devices;
};

/**
 * \brief A libusb device handle which can be used to communicate with a device.
 */
class DeviceHandle final : public NonCopyable
{
   public:
    /**
     * \brief Opens a handle to a device based on its vendor ID, product ID,
     * and, optionally, serial number.
     *
     * \param[in] context the context in which to open the handle
     *
     * \param[in] vendor_id the vendor ID of the device to open
     *
     * \param[in] product_id the product ID of the device to open
     *
     * \param[in] serial_number the serial number of the device to open, or null
     * to open a device with matching vendor and product ID but any serial
     * number
     */
    explicit DeviceHandle(
        Context &context, unsigned int vendor_id, unsigned int product_id,
        const char *serial_number = nullptr);

    /**
     * \brief Opens a handle to a specific device.
     *
     * \param[in] device the device to open
     */
    explicit DeviceHandle(const Device &device);

    /**
     * \brief Closes the device handle.
     *
     * If any outstanding asynchronous transfers are in progress, the destructor
     * blocks until they complete.
     */
    ~DeviceHandle();

    /**
     * \brief Issues a bus reset to the device.
     *
     * The device handle should be closed after the reset is issued.
     */
    void reset();

    /**
     * \brief Returns the device descriptor from the device.
     *
     * \return the device descriptor
     */
    const libusb_device_descriptor &device_descriptor() const;

    /**
     * \brief Returns a configuration descriptor from the device.
     *
     * \param[in] index the zero-based index of the descriptor to return
     *
     * \return the configuration descriptor
     */
    const libusb_config_descriptor &configuration_descriptor(
        uint8_t index) const;

    /**
     * \brief Returns a configuration descriptor from the device.
     *
     * \param[in] value the configuration value for the descriptor
     *
     * \return the configuration descriptor
     */
    const libusb_config_descriptor &configuration_descriptor_by_value(
        uint8_t value) const;

    /**
     * \brief Reads a string descriptor from the device.
     *
     * \param[in] index the index of the string descriptor to read
     *
     * \return the descriptor
     */
    std::string string_descriptor(uint8_t index) const;

    /**
     * \brief Returns the current configuration number.
     *
     * \return the configuration
     */
    int get_configuration() const;

    /**
     * \brief Sets the device’s configuration.
     *
     * \param[in] config the configuration number to set
     */
    void set_configuration(int config);

    /**
     * \brief Locks an interface so no other software can use it.
     *
     * \param[in] interface the interface number to claim
     */
    void claim_interface(int interface);

    /**
     * \brief Releases a locked interface.
     *
     * \param[in] interface the interface to release
     */
    void release_interface(int interface);

    /**
     * \brief Sets an interface into a particular alternate setting.
     *
     * \param[in] interface the interface to affect, which should be claimed
     *
     * \param[in] alternate_setting the alternate setting to switch the
     * interface into
     */
    void set_interface_alt_setting(int interface, int alternate_setting);

    /**
     * \brief Attempts to clear halt status on an IN endpoint.
     *
     * \param[in] endpoint the endpoint number to operate on
     */
    void clear_halt_in(unsigned char endpoint);

    /**
     * \brief Attempts to clear halt status on an OUT endpoint.
     *
     * \param[in] endpoint the endpoint number to operate on
     */
    void clear_halt_out(unsigned char endpoint);

    /**
     * \brief Synchronously executes a control transfer with no data stage.
     *
     * \param[in] request_type the request type field of the setup transaction
     *
     * \param[in] request the request field of the setup transaction
     *
     * \param[in] value the value field of the setup transaction
     *
     * \param[in] index the index field of the setup transaction
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    void control_no_data(
        uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
        unsigned int timeout);

    /**
     * \brief Synchronously executes a control transfer with an inbound data
     * stage.
     *
     * \param[in] request_type the request type field of the setup transaction
     *
     * \param[in] request the request field of the setup transaction
     *
     * \param[in] value the value field of the setup transaction
     *
     * \param[in] index the index field of the setup transaction
     *
     * \param[out] buffer a buffer in which to store the received data
     *
     * \param[in] len the maximum number of bytes to accept from the device
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     *
     * \return the number of bytes actually sent by the device
     */
    std::size_t control_in(
        uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
        void *buffer, std::size_t len, unsigned int timeout);

    /**
     * \brief Synchronously executes a control transfer with an outbound data
     * stage.
     *
     * \param[in] request_type the request type field of the setup transaction
     *
     * \param[in] request the request field of the setup transaction
     *
     * \param[in] value the value field of the setup transaction
     *
     * \param[in] index the index field of the setup transaction
     *
     * \param[in] buffer the data to send in the data stage
     *
     * \param[in] len the number of bytes to send in the data stage
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    void control_out(
        uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
        const void *buffer, std::size_t len, unsigned int timeout);

    /**
     * \brief Synchronously executes an inbound transfer from an interrupt
     * endpoint.
     *
     * \param[in] endpoint the endpoint number on which to transfer
     *
     * \param[out] data a buffer in which to store the received data
     *
     * \param[in] length the maximum number of bytes to accept from the device
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     *
     * \return the number of bytes actually sent by the device
     */
    std::size_t interrupt_in(
        unsigned char endpoint, void *data, std::size_t length,
        unsigned int timeout);

    /**
     * \brief Synchronously executes an outbound transfer to an interrupt
     * endpoint.
     *
     * \param[in] endpoint the endpoint number on which to transfer
     *
     * \param[in] data the data to send
     *
     * \param[in] length the number of bytes to send
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    void interrupt_out(
        unsigned char endpoint, const void *data, std::size_t length,
        unsigned int timeout);

    /**
     * \brief Synchronously executes an inbound transfer from a bulk endpoint.
     *
     * \param[in] endpoint the endpoint number on which to transfer
     *
     * \param[out] data a buffer in which to store the received data
     *
     * \param[in] length the maximum number of bytes to accept from the device
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     *
     * \return the number of bytes actually sent by the device
     */
    std::size_t bulk_in(
        unsigned char endpoint, void *data, std::size_t length,
        unsigned int timeout);

    /**
     * \brief Mark the device as shutting down, so cancelled transfers will not
     * issue warnings.
     */
    void mark_shutting_down();

   private:
    friend class Transfer;
    friend class ControlNoDataTransfer;
    friend class ControlInTransfer;
    friend class InterruptOutTransfer;
    friend class InterruptInTransfer;
    friend class BulkOutTransfer;
    friend class BulkInTransfer;
    friend void usb_transfer_handle_completed_transfer_trampoline(
        libusb_transfer *transfer);

    libusb_context *context;
    libusb_device_handle *handle;
    libusb_device_descriptor device_descriptor_;
    std::vector<std::unique_ptr<
        libusb_config_descriptor, void (*)(libusb_config_descriptor *)>>
        config_descriptors;
    unsigned int submitted_transfer_count;
    bool shutting_down;

    void init_descriptors();
};

/**
 * \brief An RAII-style way to set a configuration on a device.
 */
class ConfigurationSetter final : public NonCopyable
{
   public:
    /**
     * \brief Sets a device’s configuration.
     *
     * \param[in] device the device to set
     *
     * \param[in] configuration the configuration number to set
     */
    explicit ConfigurationSetter(DeviceHandle &device, int configuration);

    /**
     * \brief Resets the device to its original configuration.
     */
    ~ConfigurationSetter();

   private:
    DeviceHandle &device;
    int original_configuration;
};

/**
 * \brief An RAII-style way to claim an interface on a device.
 */
class InterfaceClaimer final : public NonCopyable
{
   public:
    /**
     * \brief Claims an interface on a device.
     *
     * \param[in] device the device whose interface should be claimed
     *
     * \param[in] interface the interface to claim
     */
    explicit InterfaceClaimer(DeviceHandle &device, int interface);

    /**
     * \brief Releases the interface.
     */
    ~InterfaceClaimer();

   private:
    DeviceHandle &device;
    int interface;
};

/**
 * \brief A libusb transfer.
 */
class Transfer : public AsyncOperation<void>
{
   public:
    /**
     * \brief Destroys the transfer.
     *
     * If the transfer is still executing, it will be cancelled.
     */
    virtual ~Transfer();

    /**
     * \brief Sets whether stalled transfers will be retried.
     *
     * By default, stalled transfers are retried a number of times before
     * failing as transfers can very occasionally stall spuriously.
     *
     * \param[in] retry \c true to retry stalled transfers, or \c false to fail
     * on the first stall
     */
    void retry_on_stall(bool retry)
    {
        retry_on_stall_ = retry;
    }

    /**
     * \brief Checks the outcome of the transfer.
     *
     * If the transfer failed, a corresponding exception will be thrown.
     */
    void result() const override;

    /**
     * \brief Starts the transfer.
     */
    void submit();

   protected:
    friend void usb_transfer_handle_completed_transfer_trampoline(
        libusb_transfer *transfer);

    DeviceHandle &device;
    libusb_transfer *transfer;
    bool submitted_, done_;
    bool retry_on_stall_;
    unsigned int stall_retries_left;

    explicit Transfer(DeviceHandle &dev);
    void handle_completed_transfer();
};

/**
 * \brief A libusb control transfer with no data.
 */
class ControlNoDataTransfer final : public Transfer
{
   public:
    /**
     * \brief Constructs a new transfer.
     *
     * \param[in] dev the device to which to send the request
     *
     * \param[in] request_type the request type field of the setup transaction
     *
     * \param[in] request the request field of the setup transaction
     *
     * \param[in] value the value field of the setup transaction
     *
     * \param[in] index the index field of the setup transaction
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    explicit ControlNoDataTransfer(
        DeviceHandle &dev, uint8_t request_type, uint8_t request,
        uint16_t value, uint16_t index, unsigned int timeout);
};

/**
 * \brief A libusb control transfer with inbound data.
 */
class ControlInTransfer final : public Transfer
{
   public:
    /**
     * \brief Constructs a new transfer.
     *
     * \param[in] dev the device to which to send the request
     *
     * \param[in] request_type the request type field of the setup transaction
     *
     * \param[in] request the request field of the setup transaction
     *
     * \param[in] value the value field of the setup transaction
     *
     * \param[in] index the index field of the setup transaction
     *
     * \param[in] len the maximum number of bytes to receive
     *
     * \param[in] exact_len \c true to consider the transfer a failure if it
     * transfers fewer than the requested number of bytes, or \c false to
     * consider such a result successful
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    explicit ControlInTransfer(
        DeviceHandle &dev, uint8_t request_type, uint8_t request,
        uint16_t value, uint16_t index, std::size_t len, bool exact_len,
        unsigned int timeout);

    /**
     * \brief Returns the received data.
     *
     * \return the received data
     */
    const uint8_t *data() const
    {
        assert(done_);
        return libusb_control_transfer_get_data(transfer);
    }

    /**
     * \brief Returns the number of received bytes.
     *
     * \return the size of the data
     */
    std::size_t size() const
    {
        assert(done_);
        return static_cast<std::size_t>(transfer->actual_length);
    }
};

/**
 * \brief A libusb inbound interrupt transfer.
 */
class InterruptInTransfer final : public Transfer
{
   public:
    /**
     * \brief Constructs a new transfer.
     *
     * \param[in] dev the device from which to receive data
     *
     * \param[in] endpoint the endpoint number on which to transfer data
     *
     * \param[in] len the maximum number of bytes to receive
     *
     * \param[in] exact_len \c true to consider the transfer a failure if it
     * transfers fewer than the requested number of bytes, or \c false to
     * consider such a result successful
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    explicit InterruptInTransfer(
        DeviceHandle &dev, unsigned char endpoint, std::size_t len,
        bool exact_len, unsigned int timeout);

    /**
     * \brief Returns the received data.
     *
     * \return the received data
     */
    const uint8_t *data() const
    {
        assert(done_);
        return transfer->buffer;
    }

    /**
     * \brief Returns the number of received bytes.
     *
     * \return the size of the data
     */
    std::size_t size() const
    {
        assert(done_);
        return static_cast<std::size_t>(transfer->actual_length);
    }
};

/**
 * \brief A libusb outbound interrupt transfer.
 */
class InterruptOutTransfer final : public Transfer
{
   public:
    /**
     * \brief Constructs a new transfer.
     *
     * \param[in] dev the device to which to send data
     *
     * \param[in] endpoint the endpoint number on which to send data
     *
     * \param[in] data the data to send, which is copied internally before the
     * constructor returns
     *
     * \param[in] len the number of bytes to send
     *
     * \param[in] max_len the maximum number of bytes the device is expecting to
     * receive, which is used to compute whether a zero-length packet is needed
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    explicit InterruptOutTransfer(
        DeviceHandle &dev, unsigned char endpoint, const void *data,
        std::size_t len, std::size_t max_len, unsigned int timeout);
};

/**
 * \brief A libusb inbound bulk transfer.
 */
class BulkInTransfer final : public Transfer
{
   public:
    /**
     * \brief Constructs a new transfer.
     *
     * \param[in] dev the device from which to receive data
     *
     * \param[in] endpoint the endpoint number on which to transfer data
     *
     * \param[in] len the maximum number of bytes to receive
     *
     * \param[in] exact_len \c true to consider the transfer a failure if it
     * transfers fewer than the requested number of bytes, or \c false to
     * consider such a result successful
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    explicit BulkInTransfer(
        DeviceHandle &dev, unsigned char endpoint, std::size_t len,
        bool exact_len, unsigned int timeout);

    /**
     * \brief Returns the received data.
     *
     * \return the received data
     */
    const uint8_t *data() const
    {
        assert(done_);
        return transfer->buffer;
    }

    /**
     * \brief Returns the number of received bytes.
     *
     * \return the size of the data
     */
    std::size_t size() const
    {
        assert(done_);
        return static_cast<std::size_t>(transfer->actual_length);
    }
};

/**
 * \brief A libusb outbound bulk transfer.
 */
class BulkOutTransfer final : public Transfer
{
   public:
    /**
     * \brief Constructs a new transfer.
     *
     * \param[in] dev the device to which to send data
     *
     * \param[in] endpoint the endpoint number on which to send data
     *
     * \param[in] data the data to send, which is copied internally before the
     * constructor returns
     *
     * \param[in] len the number of bytes to send
     *
     * \param[in] max_len the maximum number of bytes the device is expecting to
     * receive, which is used to compute whether a zero-length packet is needed
     *
     * \param[in] timeout the maximum length of time to let the transfer run, in
     * milliseconds, or zero for no timeout
     */
    explicit BulkOutTransfer(
        DeviceHandle &dev, unsigned char endpoint, const void *data,
        std::size_t len, std::size_t max_len, unsigned int timeout);
};
}

#endif
