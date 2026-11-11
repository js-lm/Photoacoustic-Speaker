#pragma once

#define CFG_TUSB_RHPORT0_MODE                       OPT_MODE_DEVICE

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                                 OPT_OS_PICO
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG                              0
#endif

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN                          __attribute__((aligned(4)))

#define CFG_TUD_ENDPOINT0_SIZE                      64

#define CFG_TUD_AUDIO                               1
#define CFG_TUD_CDC                                 1
#define CFG_TUD_MSC                                 0
#define CFG_TUD_HID                                 0
#define CFG_TUD_MIDI                                0
#define CFG_TUD_VENDOR                              0

// cdc (stdio). a large tx buffer so a burst of debug lines never blocks the audio loop
#define CFG_TUD_CDC_RX_BUFSIZE                      64
#define CFG_TUD_CDC_TX_BUFSIZE                      1024
#define CFG_TUD_CDC_EP_BUFSIZE                      64

// audio, keep in sync with constants::audio
#define USB_AUDIO_SAMPLE_RATE                       48000
#define USB_AUDIO_CHANNEL_COUNT                     2
#define USB_AUDIO_BYTES_PER_SAMPLE                  2
#define USB_AUDIO_BITS_PER_SAMPLE                   16

#define USB_AUDIO_SPEAKER_DESCRIPTOR_LENGTH ( \
      TUD_AUDIO_DESC_IAD_LEN \
    + TUD_AUDIO_DESC_STD_AC_LEN \
    + TUD_AUDIO_DESC_CS_AC_LEN \
    + TUD_AUDIO_DESC_CLK_SRC_LEN \
    + TUD_AUDIO_DESC_INPUT_TERM_LEN \
    + TUD_AUDIO_DESC_OUTPUT_TERM_LEN \
    + TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN \
    + TUD_AUDIO_DESC_CS_AS_INT_LEN \
    + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN \
    + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN \
    + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN \
    + TUD_AUDIO_DESC_STD_AS_ISO_FB_EP_LEN)

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN               USB_AUDIO_SPEAKER_DESCRIPTOR_LENGTH
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT               1
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ            64

#define CFG_TUD_AUDIO_ENABLE_EP_OUT                 1
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX          USB_AUDIO_CHANNEL_COUNT
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX  USB_AUDIO_BYTES_PER_SAMPLE
#define CFG_TUD_AUDIO_FUNC_1_RESOLUTION_RX          USB_AUDIO_BITS_PER_SAMPLE
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX          TUD_AUDIO_EP_SIZE(USB_AUDIO_SAMPLE_RATE, USB_AUDIO_BYTES_PER_SAMPLE, USB_AUDIO_CHANNEL_COUNT)

// 8 packets (about 8 ms). the fifo count feedback regulates this to half full, which is the usb side latency
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ       (8 * CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX)

// asynchronous sink: the pico clock sets the rate, the host follows the feedback (16.16, linux and windows)
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP                1
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_FORMAT_CORRECTION 0
