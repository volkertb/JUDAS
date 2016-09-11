
/*
 * Internal header file: simplified definitions (used by AC97/HDA code)
 * Some parts of the header file are based on ALSA HDA defines, some on common
 * Intel docs HDA defines. Almost everything has been rewritten,  the whole
 * stuff was simplified and adapted to support the Judas library.
 * Please be aware that HDA support functions fall uder the GNU/GPL license.
 *
 * by Piotr Ulaszewski (PETERS)
 * March 2008 rev 2.1/2,2
 */

#define FALSE          0
#define TRUE           1
#ifdef  NULL           // override NULL to 0
#undef  NULL
#endif
#define NULL           0
typedef int            bool;
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned int   UINT;
typedef unsigned long  ULONG;
typedef DWORD          dword;
typedef WORD           word;
typedef BYTE           byte;
#define LOWORD(l)      ((WORD)((DWORD)(l)))
#define HIWORD(l)      ((WORD)(((DWORD)(l)>>16)&0xFFFF))
#define LOBYTE(w)      ((BYTE)(w))
#define HIBYTE(w)      ((BYTE)(((WORD)(w)>>8)&0xFF))


#define BIT0           1
#define BIT1           2
#define BIT2           4
#define BIT3           8
#define BIT4           0x10
#define BIT5           0x20
#define BIT6           0x40
#define BIT7           0x80
#define BIT8           0x100
#define BIT9           0x200
#define BIT10          0x400
#define BIT11          0x800
#define BIT12          0x1000
#define BIT13          0x2000
#define BIT14          0x4000
#define BIT15          0x8000
#define BIT16          0x10000
#define BIT17          0x20000
#define BIT18          0x40000
#define BIT19          0x80000
#define BIT20          0x100000
#define BIT21          0x200000
#define BIT22          0x400000
#define BIT23          0x800000
#define BIT24          0x1000000
#define BIT25          0x2000000
#define BIT26          0x4000000
#define BIT27          0x8000000
#define BIT28          0x10000000
#define BIT29          0x20000000
#define BIT30          0x40000000
#define BIT31          0x80000000


/*****************************************************************************
 * General PCI static defines
 ******************************************************************************/

#define PCI_CLASS_MULTIMEDIA_AUDIO  0x0401
#define PCI_CLASS_MULTIMEDIA_OTHER  0x0480
#define PCI_VENDER_ID               0x00
#define PCI_REVISION_ID             0x08
#define PCI_COMMAND                 0x04
#define PCI_DEVICE_ID               0x02
#define PCI_INTERRUPT_LINE          0x3c
#define PCI_INT_LINE                0x3d

#define PCI_MEM_BASE_ADDRESS_0      0x10
#define PCI_MEM_BASE_ADDRESS_1      0x14
#define PCI_MEM_BASE_ADDRESS_2      0x18
#define PCI_MEM_BASE_ADDRESS_3      0x1c

#define PCI_BASE_ADDRESS_0          0x10
#define PCI_BASE_ADDRESS_1          0x14
#define PCI_BASE_ADDRESS_2          0x18
#define PCI_BASE_ADDRESS_3          0x1c
#define PCI_BASE_ADDRESS_4          0x20
#define PCI_BASE_ADDRESS_5          0x24

#define PCI_COMMAND_IO              0x01
#define PCI_COMMAND_MEMORY          0x02
#define PCI_COMMAND_MASTER          0x04
#define PCI_COMMAND_PARITY          0x40
#define PCI_ICH4_CFG_REG            0x41
#define PCI_COMMAND_SERR            0x100

#define PCI_STATUS                  0x06
#define PCI_SUBSYSTEM_VENDER_ID     0x2c
#define PCI_SUBSYSTEM_ID            0x2e

/* Ac97 general defines */
#define CTL_BASE                    0     /* addressing controller regs */
#define MIXER_BASE                  1     /* addressing mixer regs */

/*****************************************************************************
 * PCI space extended defines HDA specific
 ******************************************************************************/

/* PCI space */
#define HDA_PCIREG_TCSEL                    0x44

/* Defines for ATI HD Audio support in SB450 south bridge */
#define ATI_SB450_HDAUDIO_MISC_CNTR2_ADDR   0x42
#define ATI_SB450_HDAUDIO_ENABLE_SNOOP      0x02

/* Defines for Nvidia HDA support */
#define NVIDIA_HDA_TRANSREG_ADDR            0x4e
#define NVIDIA_HDA_ENABLE_COHBITS           0x0f

/* HDA general defines */
#define HDA_MAX_DEV                         16
#define HDA_MAX_CODECS                      4
#define HDA_MAX_CODEC_ADDRESS               0x0f

/* max. connections to a widget */
#define HDA_MAX_CONNECTIONS                 32
#define HDA_MAX_PCM_VOLS                    2
#define HDA_MAX_NUM_INPUTS                  16


/*******************************************************
 * Internal header file: Intel ICH AC97/HDA and compatibles.
 * (AC97/HDA pci device structure and defines)
 *******************************************************/

typedef struct {
        WORD vender_id;
        WORD device_id;
        WORD sub_vender_id;
        WORD sub_device_id;
        int  type;
        char *string;
} AUDIO_DEVICE_LIST;

typedef struct {
        char *string;
} AUDIO_STEREO_TECHNOLOGY;

typedef struct {
        WORD vender_id1;
        WORD vender_id2;
        char *string;
} AUDIO_CODEC_LIST;


/**********************************************************/
/*                        The main AC97/HDA Audio structure                        */
/**********************************************************/

#pragma pack(push,1)
// 1 HDA node with it's data
struct hda_node {
        WORD nid;                   /* NID of this widget */
        WORD nconns;                /* number of input connections */
        WORD conn_list[HDA_MAX_CONNECTIONS];
        DWORD wid_caps;             /* widget capabilities */
        DWORD type;                 /* node/widget type - bits 20 - 23 of wid_caps */
        DWORD pin_caps;             /* pin widget capabilities */
        DWORD pin_ctl;              /* pin controls */
        DWORD def_config;           /* default configuration */
        DWORD amp_out_caps;         /* AMP out capabilities override over default */
        DWORD amp_in_caps;          /* AMP in capabilities override over default */
        DWORD supported_formats;    /* supported formats value */
        BYTE checked;               /* flag indicator that node is already parsed */
};

struct pcm_vol_str {
 struct hda_node *node;             /* Node for PCM volume */
        DWORD index;                /* connection of PCM volume */
};

 typedef struct {
        // PCI device IDs
        WORD vender_id;
        WORD device_id;
        WORD sub_vender_id;
        WORD sub_device_id;
        WORD device_bus_number;
        BYTE irq;                   //PCI IRQ
        BYTE pin;                   // PCI IRQ PIN
        WORD command;               // PCI command reg
        DWORD base0;                // PCI BAR for mixer registers NAMBAR_REG
        DWORD base1;                // PCI BAR for bus master registers NABMBAR_REG
        DWORD base2;
        DWORD base3;
        DWORD base4;
        DWORD base5;
        DWORD device_type;          // any

        // driver type flags
        int mem_mode;               // 0 for IO access, 1 for memory access
        int hda_mode;               // 0 for AC97 mode, 1 for HDA mode

        // memory allocated for BDL and PCM buffers
        DWORD *bdl_buffer;          // Buffer Descriptor List for AC97 - 256 bytes / HDA - 8192 bytes
        DWORD *pcmout_buffer0;      // Low DOS memory AC97/HDA buffer 0 for Bus Master DMA
        DWORD *pcmout_buffer1;      // Low DOS memory AC97/HDA buffer 1 for Bus Master DMA
        DWORD *hda_buffer;          // base of HDA allocated memory non aligned
        DWORD pcmout_bufsize;       // size of PCM out buffer - obsolete
        DWORD pcmout_bdl_entries;   // number of BDL entries - obsolete
        DWORD pcmout_bdl_size;      // single BDL size - obsolete
        DWORD pcmout_dmasize;         // size of 1 BDL entry - obsolete
        DWORD pcmout_dma_lastgoodpos; // last good position in DMA - obsolete
        DWORD pcmout_dma_pos_ptr;     // buffer for DMA position (not used in Judas) - obsolete

        // ac97 only properties
        int ac97_vra_supported;    // False by default

        // HDA only properties        
        unsigned long codec_mask;                  // mask for all available codecs
        unsigned int codec_index;                  // 1st codec that passed hardware init

        unsigned short afg_root_nodenum;           // Audio Function Group root node
        int afg_num_nodes;                         // number of subordinate nodes connected to AFG root node
        struct hda_node *afg_nodes;                // all nodes connected to root AFG node
        unsigned int def_amp_out_caps;             // default out amplifier capabilities
        unsigned int def_amp_in_caps;              // default in amplifier capabilities

        struct hda_node *dac_node[2];	           // DAC nodes
        struct hda_node *out_pin_node[2];          // Output pin nodes - all (Line-Out/hp-out, etc...)
        struct hda_node *adc_node[2];              // ADC nodes
        struct hda_node *in_pin_node[2];           // Input pin nodes - all (CD-In, etc...) - obsolete
        unsigned int input_items;                  // Input items for capture  
        unsigned int pcm_num_vols;	               // number of PCM volumes
        struct pcm_vol_str pcm_vols[HDA_MAX_PCM_VOLS]; // PCM volume nodes

        unsigned int  format_val;                  // stream type
        unsigned int  dacout_num_bits;             // bits for playback (16 bits by default)
        unsigned int  dacout_num_channels;         // channels for playback (2 = stereo by default)
        unsigned int  stream_tag;                  // stream associated with our SD (1 by default)
        unsigned long supported_formats;
        unsigned long supported_max_freq;
        unsigned int  supported_max_bits;
        
        unsigned long freq_card;                   // current frequency 44100 by default
        unsigned int  chan_card;
        unsigned int  bits_card;

        // codec IDs and names
        WORD codec_id1;             // codec vender id
        WORD codec_id2;             // codec device id
        char device_name[128];      // controller name string
        char codec_name[128];       // codec name string
} AUDIO_PCI_DEV;
#pragma pack(pop)


/**********************************************************/
/*                                AC97 base0 commands                                         */
/* registers accessed via NAMBAR - base0 - Audio Mixer registers */
/*                                                                                                                */
/**********************************************************/

#define AC97_RESET                      0x00    // Reset register
#define AC97_MASTER                     0x02    // Master Volume
#define AC97_HEADPHONE                  0x04    // Headphone Volume (optional)
#define AC97_MASTER_MONO                0x06
#define AC97_MASTER_TONE                0x08
#define AC97_PCBEEP_VOL                 0x0a
#define AC97_PHONE_VOL                  0x0c
#define AC97_MIC_VOL                    0x0e
#define AC97_LINE_IN_VOL                0x10
#define AC97_CD_VOL                     0x12
#define AC97_VID_VOL                    0x14
#define AC97_AUX_VOL                    0x16
#define AC97_PCM                        0x18    // PCM OUT Volume
#define AC97_RECORD_SELECT              0x1a
#define AC97_RECORD_VOL                 0x1c
#define AC97_RECORD_MIC_VOL             0x1e
#define AC97_GP_REG                     0x20    // general purpose
#define AC97_3D_CONTROL_REG             0x22    // 3D control
                                                // 24h is reserved
#define AC97_POWER_CTRL                 0x26    // powerdown control
#define AC97_EXTENDED_ID                0x28    // Extended Audio ID
#define AC97_EXTENDED_STATUS            0x2a    // Extended Audio Status (control)
#define AC97_PCM_FRONT_DAC_RATE         0x2c    // PCM OUT Front DAC Rate
#define AC97_PCM_SURND_DAC_RATE         0x2e    // surround sound sample rate
#define AC97_PCM_LFE_DAC_RATE           0x30    // LFE samplerate
#define AC97_LR_ADC_DAC_RATE            0x32    // pcm in sample rate
#define AC97_MIC_ADC_RATE               0x34    // mic in sample rate
                                                //registers 36-7a are reserved on the ICH
#define AC97_VENDERID1_REG              0x7c    // codec vender ID 1
#define AC97_VENDERID2_REG              0x7e    // codec vender ID 2

// When 2 codecs are present in the system, use BIT7 to access the 2nd set of registers, ie 80h-feh

#define PRIMARY_CODEC                   0       // 0-7F for primary codec
#define SECONDARY_CODEC                 BIT7    // 80-8f registers for 2ndary

#define AC97_EA_VRA                     BIT0    /* Variable Rate Audio enable bit */
#define AC97_EA_DRA                     BIT1    /* Double Rate Audio enable bit */


/* registers accessed via NABMBAR - base1 - Bus Master registers */

/* capture block - PCM IN */
#define ICH_REG_PI_BDBAR                0x00    /* dword - Buffer Descriptor list base address */
#define ICH_REG_PI_CIV                  0x04    /* byte - Current Index Value */
#define ICH_REG_PI_LVI                  0x05    /* byte - Last Valid Index */
#define ICH_REG_PI_SR                   0x06    /* byte - Status Register */
#define ICH_REG_PI_PICB                 0x08    /* word - Position In Current Buffer */
#define ICH_REG_PI_PIV                  0x0a    /* byte - Prefetched Index Value */
#define ICH_REG_PI_CR                   0x0b    /* byte - Control Register */


/* playback block - PCM OUT */
#define ICH_REG_PO_BDBAR                0x10    /* dword - Buffer Descriptor list base address */
#define ICH_REG_PO_CIV                  0x14    /* byte - Current Index Value */
#define ICH_REG_PO_LVI                  0x15    /* byte - Last Valid Index */
#define ICH_REG_PO_SR                   0x16    /* byte - Status Register */
#define ICH_REG_PO_PICB                 0x18    /* word - Position In Current Buffer */
#define ICH_REG_PO_PIV                  0x1a    /* byte - Prefetched Index Value */
#define ICH_REG_PO_CR                   0x1b    /* byte - Control Register */


/* mic capture block - MIC IN */
#define ICH_REG_MC_BDBAR                0x20    /* dword - Buffer Descriptor list base address */
#define ICH_REG_MC_CIV                  0x24    /* byte - Current Index Value */
#define ICH_REG_MC_LVI                  0x25    /* byte - Last Valid Index */
#define ICH_REG_MC_SR                   0x26    /* byte - Status Register */
#define ICH_REG_MC_PICB                 0x28    /* word - Position In Current Buffer */
#define ICH_REG_MC_PIV                  0x2a    /* byte - Prefetched Index Value */
#define ICH_REG_MC_CR                   0x2b    /* byte - Control Register */


/* bitfields for (PCM IN/PCM OUT/MIC IN) */
#define ICH_REG_LVI_MASK                0x1f    /* LVI range mask -> 0 - 31 */
#define ICH_REG_PIV_MASK                0x1f    /* PIV range mask -> 0 - 31 */

/* status register bitfields */
#define ICH_FIFOE                       0x10    /* FIFO error (overrun/underrun) */
#define ICH_BCIS                        0x08    /* Buffer Completion Interrupt Status */
#define ICH_LVBCI                       0x04    /* Last Valid Buffer Completion Interrupt */
#define ICH_CELV                        0x02    /* Current Equals Last Valid */
#define ICH_DCH                         0x01    /* DMA Controller Halted */

/* control register bitfields */
#define ICH_IOCE                        0x10    /* Interrupt On Completion Enable */
#define ICH_FEIE                        0x08    /* Fifo Error Interrupt Enable */
#define ICH_LVBIE                       0x04    /* Last Valid Buffer Interrupt Enable */
#define ICH_RESETREGS                   0x02    /* Reset busmaster Registers */
#define ICH_STARTBM                     0x01    /* Start Busmaster operation */


/* global block - control/status */
#define ICH_REG_GLOB_CNT                0x2c    /* dword - global control register */
#define ICH_24_BIT                     BIT23    /* 24 bit samples */
#define ICH_20_BIT                     BIT22    /* 20 bit samples */
#define ICH_PCM_246_MASK       BIT20 | BIT21    /* 6 channels (not all chips) */
#define ICH_PCM_6                      BIT21    /* 6 channels (not all chips) */
#define ICH_PCM_4                      BIT20    /* 4 channels (not all chips) */
#define ICH_PCM_2                          0    /* 2 channels (stereo)  same as ICH_SIS */
#define ICH_SIS_PCM_246_MASK     BIT6 | BIT7    /* 6 channels (not all chips) */
#define ICH_SIS_PCM_6                   BIT7    /* 6 channels (not all chips) */
#define ICH_SIS_PCM_4                   BIT6    /* 4 channels (not all chips) */
#define ICH_SIS_PCM_2                      0    /* 2 channels (stereo) same as ICH */
#define ICH_SRIE                        BIT5    /* Secondary Resume Interrupt Enable */
#define ICH_PRIE                        BIT4    /* Primary Resume Interrupt Enable */
#define ICH_ACLINK                      BIT3    /* AClink shut off */
#define ICH_AC97WARM                    BIT2    /* AC97 Warm reset */
#define ICH_AC97COLD                    BIT1    /* AC97 Cold reset */
#define ICH_GIE                         BIT0    /* GPI Interrupt Enable */

#define ICH_REG_GLOB_STA                0x30    /* dword - global status register */
#define ICH_24_BIT_CAP                 BIT23    /* 24 bit sample support */
#define ICH_20_BIT_CAP                 BIT22    /* 20 bit sample support */
#define ICH_PCM_6_CAP                  BIT21    /* 6 channels capability */
#define ICH_PCM_4_CAP                  BIT20    /* 4 channels capability */
#define ICH_MD3                        BIT17    /* Modem power Down semaphore (status) */
#define ICH_AD3                        BIT16    /* Audio power Down semaphore (status) */
#define ICH_RCS                        BIT15    /* Read Completion Status (0=normal) */
#define ICH_BIT3                       BIT14    /* shadowed status of bit 3 in slot 12 */
#define ICH_BIT2                       BIT13    /* shadowed status of bit 2 in slot 12 */
#define ICH_BIT1                       BIT12    /* shadowed status of bit 1 in slot 12 */
#define ICH_SRI                        BIT11    /* Secondary codec Resume Interrupt */
#define ICH_PRI                        BIT10    /* Primary codec Resume Interrupt */
#define ICH_SCR                         BIT9    /* Secondary Codec Ready */
#define ICH_PCR                         BIT8    /* Primary Codec Ready */
#define ICH_MCINT                       BIT7    /* MIC In Interrupt - mic capture */
#define ICH_POINT                       BIT6    /* PCM Out Interrupt - pcm playback */
#define ICH_PIINT                       BIT5    /* PCM In Interrupt - pcm capture */
#define ICH_MOINT                       BIT2    /* Modem Out Interrupt - modem playback */
#define ICH_MIINT                       BIT1    /* Modem In Interrupt - modem capture */
#define ICH_GSCI                        BIT0    /* GPI Status Change Interrupt */

#define ICH_REG_ACC_SEMA                0x34    /* byte - codec write semaphore register */
#define ICH_CAS                         BIT0    /* Codec Access Semaphore */


/*****************************************************************************
 * Internal header file: Intel ICH HDA and compatibles.
 * (HDA pci device structure and defines as in Intel docs)
 ******************************************************************************/

/*
 * nodes
 */
#define	AC_NODE_ROOT                        0x00

/*
 * function group types
 */
enum {
	AC_GRP_AUDIO_FUNCTION = 0x01,
	AC_GRP_MODEM_FUNCTION = 0x02,
};
	
/*
 * widget types
 */
enum {
	AC_WID_AUD_OUT,                         /* Audio Out */
	AC_WID_AUD_IN,                          /* Audio In */
	AC_WID_AUD_MIX,                         /* Audio Mixer */
	AC_WID_AUD_SEL,                         /* Audio Selector */
	AC_WID_PIN,                             /* Pin Complex */
	AC_WID_POWER,                           /* Power */
	AC_WID_VOL_KNB,                         /* Volume Knob */
	AC_WID_BEEP,                            /* Beep Generator */
	AC_WID_VENDOR = 0x0f                    /* Vendor specific */
};

/*
 * GET verbs
 */
#define AC_VERB_GET_STREAM_FORMAT           0x0a00
#define AC_VERB_GET_AMP_GAIN_MUTE           0x0b00
#define AC_VERB_GET_PROC_COEF               0x0c00
#define AC_VERB_GET_COEF_INDEX              0x0d00
#define AC_VERB_PARAMETERS                  0x0f00
#define AC_VERB_GET_CONNECT_SEL             0x0f01
#define AC_VERB_GET_CONNECT_LIST            0x0f02
#define AC_VERB_GET_PROC_STATE              0x0f03
#define AC_VERB_GET_SDI_SELECT              0x0f04
#define AC_VERB_GET_POWER_STATE             0x0f05
#define AC_VERB_GET_CONV                    0x0f06
#define AC_VERB_GET_PIN_WIDGET_CONTROL      0x0f07
#define AC_VERB_GET_UNSOLICITED_RESPONSE    0x0f08
#define AC_VERB_GET_PIN_SENSE               0x0f09
#define AC_VERB_GET_BEEP_CONTROL            0x0f0a
#define AC_VERB_GET_EAPD_BTLENABLE          0x0f0c
#define AC_VERB_GET_DIGI_CONVERT            0x0f0d
#define AC_VERB_GET_VOLUME_KNOB_CONTROL     0x0f0f
#define AC_VERB_GET_GPIO_DATA               0x0f15
#define AC_VERB_GET_GPIO_MASK               0x0f16
#define AC_VERB_GET_GPIO_DIRECTION          0x0f17
#define AC_VERB_GET_CONFIG_DEFAULT          0x0f1c
#define AC_VERB_GET_SUBSYSTEM_ID            0x0f20

/*
 * SET verbs
 */
#define AC_VERB_SET_STREAM_FORMAT           0x200
#define AC_VERB_SET_AMP_GAIN_MUTE           0x300
#define AC_VERB_SET_PROC_COEF               0x400
#define AC_VERB_SET_COEF_INDEX              0x500
#define AC_VERB_SET_CONNECT_SEL             0x701
#define AC_VERB_SET_PROC_STATE              0x703
#define AC_VERB_SET_SDI_SELECT              0x704
#define AC_VERB_SET_POWER_STATE             0x705
#define AC_VERB_SET_CHANNEL_STREAMID        0x706
#define AC_VERB_SET_PIN_WIDGET_CONTROL      0x707
#define AC_VERB_SET_UNSOLICITED_ENABLE      0x708
#define AC_VERB_SET_PIN_SENSE               0x709
#define AC_VERB_SET_BEEP_CONTROL            0x70a
#define AC_VERB_SET_EAPD_BTLENABLE          0x70c
#define AC_VERB_SET_DIGI_CONVERT_1          0x70d
#define AC_VERB_SET_DIGI_CONVERT_2          0x70e
#define AC_VERB_SET_VOLUME_KNOB_CONTROL     0x70f
#define AC_VERB_SET_GPIO_DATA               0x715
#define AC_VERB_SET_GPIO_MASK               0x716
#define AC_VERB_SET_GPIO_DIRECTION          0x717
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_0  0x71c
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_1  0x71d
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_2  0x71e
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_3  0x71f
#define AC_VERB_SET_CODEC_RESET             0x7ff

/*
 * Parameter IDs
 */
#define AC_PAR_VENDOR_ID                    0x00
#define AC_PAR_SUBSYSTEM_ID                 0x01
#define AC_PAR_REV_ID                       0x02
#define AC_PAR_NODE_COUNT                   0x04
#define AC_PAR_FUNCTION_TYPE                0x05
#define AC_PAR_AUDIO_FG_CAP                 0x08
#define AC_PAR_AUDIO_WIDGET_CAP             0x09
#define AC_PAR_PCM                          0x0a
#define AC_PAR_STREAM                       0x0b
#define AC_PAR_PIN_CAP                      0x0c
#define AC_PAR_AMP_IN_CAP                   0x0d
#define AC_PAR_CONNLIST_LEN                 0x0e
#define AC_PAR_POWER_STATE                  0x0f
#define AC_PAR_PROC_CAP                     0x10
#define AC_PAR_GPIO_CAP                     0x11
#define AC_PAR_AMP_OUT_CAP                  0x12

/*
 * AC_VERB_PARAMETERS results (32bit)
 */

/* Function Group Type */
#define AC_FGT_TYPE                         0xff
#define AC_FGT_TYPE_SHIFT                   0
#define AC_FGT_UNSOL_CAP                    BIT8

/* Audio Function Group Capabilities */
#define AC_AFG_OUT_DELAY                    0xf
#define AC_AFG_IN_DELAY                     (0xf << 8)
#define AC_AFG_BEEP_GEN                     BIT16

/* Audio Widget Capabilities */
#define AC_WCAP_STEREO                      BIT0  /* stereo I/O */
#define AC_WCAP_IN_AMP                      BIT1  /* AMP-in present */
#define AC_WCAP_OUT_AMP                     BIT2  /* AMP-out present */
#define AC_WCAP_AMP_OVRD                    BIT3  /* AMP-parameter override */
#define AC_WCAP_FORMAT_OVRD                 BIT4  /* format override */
#define AC_WCAP_STRIPE                      BIT5  /* stripe */
#define AC_WCAP_PROC_WID                    BIT6  /* Proc Widget */
#define AC_WCAP_UNSOL_CAP                   BIT7  /* Unsol capable */
#define AC_WCAP_CONN_LIST                   BIT8  /* connection list */
#define AC_WCAP_DIGITAL                     BIT9  /* digital I/O */
#define AC_WCAP_POWER                       BIT10 /* power control */
#define AC_WCAP_LR_SWAP                     BIT11 /* L/R swap */
#define AC_WCAP_DELAY                       (0xf << 16)
#define AC_WCAP_DELAY_SHIFT                 16

/* supported PCM rates and bits */
#define AC_SUPPCM_RATES                     0xfff
#define AC_SUPPCM_BITS_8                    BIT16
#define AC_SUPPCM_BITS_16                   BIT17
#define AC_SUPPCM_BITS_20                   BIT18
#define AC_SUPPCM_BITS_24                   BIT19
#define AC_SUPPCM_BITS_32                   BIT20

/* supported PCM stream format */
#define AC_SUPFMT_PCM                       BIT0
#define AC_SUPFMT_FLOAT32                   BIT1
#define AC_SUPFMT_AC3                       BIT2

/* Pin widget capabilies */
#define AC_PINCAP_IMP_SENSE                 BIT0    /* impedance sense capable */
#define AC_PINCAP_TRIG_REQ                  BIT1    /* trigger required */
#define AC_PINCAP_PRES_DETECT               BIT2    /* presence detect capable */
#define AC_PINCAP_HP_DRV                    BIT3    /* headphone drive capable */
#define AC_PINCAP_OUT                       BIT4    /* output capable */
#define AC_PINCAP_IN                        BIT5    /* input capable */
#define AC_PINCAP_BALANCE                   BIT6    /* balanced I/O capable */
#define AC_PINCAP_VREF                      (0x37 << 8)
#define AC_PINCAP_VREF_SHIFT                8
#define AC_PINCAP_EAPD                      BIT16   /* EAPD capable */

/* Vref status (used in pin cap) */
#define AC_PINCAP_VREF_HIZ                  BIT0    /* Hi-Z */
#define AC_PINCAP_VREF_50                   BIT1    /* 50% */
#define AC_PINCAP_VREF_GRD                  BIT2    /* ground */
#define AC_PINCAP_VREF_80                   BIT3    /* 80% */
#define AC_PINCAP_VREF_100                  BIT5    /* 100% */

/* Amplifier capabilities */
#define AC_AMPCAP_OFFSET                    (0x7f << 0)  /* 0dB offset */
#define AC_AMPCAP_OFFSET_SHIFT              0
#define AC_AMPCAP_NUM_STEPS                 (0x7f << 8)  /* number of steps */
#define AC_AMPCAP_NUM_STEPS_SHIFT           8
#define AC_AMPCAP_STEP_SIZE                 (0x7f << 16) /* step size 0-32dB in 0.25dB */
#define AC_AMPCAP_STEP_SIZE_SHIFT           16
#define AC_AMPCAP_MUTE                      BIT31        /* mute capable */
#define AC_AMPCAP_MUTE_SHIFT                31

/* Supported power status */
#define AC_PWRST_D0SUP                      BIT0
#define AC_PWRST_D1SUP                      BIT1
#define AC_PWRST_D2SUP                      BIT2
#define AC_PWRST_D3SUP                      BIT3

/* Power state values */
#define AC_PWRST_D0                         0x00
#define AC_PWRST_D1	                        0x01
#define AC_PWRST_D2                         0x02
#define AC_PWRST_D3	                        0x03

/* Processing capabilies */
#define AC_PCAP_BENIGN                      BIT0
#define AC_PCAP_NUM_COEF                    (0xff << 8)

/* Volume knobs capabilities */
#define AC_KNBCAP_NUM_STEPS                 0x7f
#define AC_KNBCAP_DELTA                     BIT8

/*
 * Control Parameters
 */

/* Amplifier gain/mute */
#define AC_AMP_MUTE                         BIT7
#define AC_AMP_GAIN                         BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6
#define AC_AMP_GET_INDEX                    BIT0 | BIT1 | BIT2 | BIT3

#define AC_AMP_GET_LEFT                     BIT13
#define AC_AMP_GET_RIGHT                    0   /* bit13 clear */
#define AC_AMP_GET_OUTPUT                   BIT15
#define AC_AMP_GET_INPUT                    0   /* bit15 clear */

#define AC_AMP_SET_INDEX                    BIT8 | BIT9 | BIT10 | BIT11
#define AC_AMP_SET_RIGHT                    BIT12
#define AC_AMP_SET_LEFT                     BIT13
#define AC_AMP_SET_INPUT                    BIT14
#define AC_AMP_SET_OUTPUT                   BIT15

/* DIGITAL1 bits */
#define AC_DIG1_ENABLE                      BIT0
#define AC_DIG1_V                           BIT1
#define AC_DIG1_VCFG                        BIT2
#define AC_DIG1_EMPHASIS                    BIT3
#define AC_DIG1_COPYRIGHT                   BIT4
#define AC_DIG1_NONAUDIO                    BIT5
#define AC_DIG1_PROFESSIONAL                BIT6
#define AC_DIG1_LEVEL                       BIT7

/* Pin widget control - 8bit */
#define AC_PINCTL_VREFEN                    0x7
#define AC_PINCTL_VREF_HIZ                  0  /* Hi-Z */
#define AC_PINCTL_VREF_50                   1  /* 50% */
#define AC_PINCTL_VREF_GRD                  2  /* ground */
#define AC_PINCTL_VREF_80                   4  /* 80% */
#define AC_PINCTL_VREF_100                  5  /* 100% */
#define AC_PINCTL_IN_EN                     BIT5
#define AC_PINCTL_OUT_EN                    BIT6
#define AC_PINCTL_HP_EN                     BIT7

/* Unsolicited response - 8bit */
#define AC_USRSP_EN                         BIT7

/* configuration default - 32bit */
#define AC_DEFCFG_SEQUENCE                  (0xf << 0)
#define AC_DEFCFG_DEF_ASSOC                 (0xf << 4)
#define AC_DEFCFG_ASSOC_SHIFT               4
#define AC_DEFCFG_MISC                      (0xf << 8)
#define AC_DEFCFG_MISC_SHIFT                8
#define AC_DEFCFG_COLOR                     (0xf << 12)
#define AC_DEFCFG_COLOR_SHIFT               12
#define AC_DEFCFG_CONN_TYPE                 (0xf << 16)
#define AC_DEFCFG_CONN_TYPE_SHIFT           16
#define AC_DEFCFG_DEVICE                    (0xf << 20)
#define AC_DEFCFG_DEVICE_SHIFT              20
#define AC_DEFCFG_LOCATION                  (0x3f << 24)
#define AC_DEFCFG_LOCATION_SHIFT            24
#define AC_DEFCFG_PORT_CONN                 (0x3 << 30)
#define AC_DEFCFG_PORT_CONN_SHIFT           30

/* device types (0x0 - 0xf) */
enum {
	AC_JACK_LINE_OUT,
	AC_JACK_SPEAKER,
	AC_JACK_HP_OUT,
	AC_JACK_CD,
	AC_JACK_SPDIF_OUT,
	AC_JACK_DIG_OTHER_OUT,
	AC_JACK_MODEM_LINE_SIDE,
	AC_JACK_MODEM_HAND_SIDE,
	AC_JACK_LINE_IN,
	AC_JACK_AUX,
	AC_JACK_MIC_IN,
	AC_JACK_TELEPHONY,
	AC_JACK_SPDIF_IN,
	AC_JACK_DIG_OTHER_IN,
	AC_JACK_OTHER = 0xf,
};

/* jack connection types (0x0 - 0xf) */
enum {
	AC_JACK_CONN_UNKNOWN,
	AC_JACK_CONN_1_8,
	AC_JACK_CONN_1_4,
	AC_JACK_CONN_ATAPI,
	AC_JACK_CONN_RCA,
	AC_JACK_CONN_OPTICAL,
	AC_JACK_CONN_OTHER_DIGITAL,
	AC_JACK_CONN_OTHER_ANALOG,
	AC_JACK_CONN_DIN,
	AC_JACK_CONN_XLR,
	AC_JACK_CONN_RJ11,
	AC_JACK_CONN_COMB,
	AC_JACK_CONN_OTHER = 0xf,
};

/* jack colors (0x0 - 0xf) */
enum {
	AC_JACK_COLOR_UNKNOWN,
	AC_JACK_COLOR_BLACK,
	AC_JACK_COLOR_GREY,
	AC_JACK_COLOR_BLUE,
	AC_JACK_COLOR_GREEN,
	AC_JACK_COLOR_RED,
	AC_JACK_COLOR_ORANGE,
	AC_JACK_COLOR_YELLOW,
	AC_JACK_COLOR_PURPLE,
	AC_JACK_COLOR_PINK,
	AC_JACK_COLOR_WHITE = 0xe,
	AC_JACK_COLOR_OTHER,
};

/* Jack location (0x0 - 0x3f) */
enum {
	AC_JACK_LOC_NONE,
	AC_JACK_LOC_REAR,
	AC_JACK_LOC_FRONT,
	AC_JACK_LOC_LEFT,
	AC_JACK_LOC_RIGHT,
	AC_JACK_LOC_TOP,
	AC_JACK_LOC_BOTTOM,
};

/* bits 4-5 */
enum {
	AC_JACK_LOC_EXTERNAL = 0x00,
	AC_JACK_LOC_INTERNAL = 0x10,
	AC_JACK_LOC_SEPARATE = 0x20,
	AC_JACK_LOC_OTHER    = 0x30,
};

enum {
	/* external on primary chasis */
	AC_JACK_LOC_REAR_PANEL = 0x07,
	AC_JACK_LOC_DRIVE_BAY,
	/* internal */
	AC_JACK_LOC_RISER = 0x17,
	AC_JACK_LOC_HDMI,
	AC_JACK_LOC_ATAPI,
	/* others */
	AC_JACK_LOC_MOBILE_IN = 0x37,
	AC_JACK_LOC_MOBILE_OUT,
};

/* Port connectivity (0-3) */
enum {
	AC_JACK_PORT_COMPLEX,
	AC_JACK_PORT_NONE,
	AC_JACK_PORT_FIXED,
	AC_JACK_PORT_BOTH,
};

/* 0 = input, 1 = output */
enum {
	HDA_INPUT, HDA_OUTPUT
};

/* amp values */
#define AMP_IN_MUTE(idx)              (0x7080 | ((idx)<< 8))
#define AMP_IN_UNMUTE(idx)            (0x7000 | ((idx)<< 8))
#define AMP_OUT_MUTE                  0xb080
#define AMP_OUT_UNMUTE                0xb000
#define AMP_OUT_ZERO                  0xb000

/* pinctl values */
#define PIN_IN                        0x20
#define PIN_VREF80                    0x24
#define PIN_VREF50                    0x21
#define PIN_OUT                       0x40
#define PIN_HP                        0xc0
#define PIN_HP_AMP                    0x80

#define defconfig_type(node) (((node)->def_config & AC_DEFCFG_DEVICE) >> AC_DEFCFG_DEVICE_SHIFT)
#define defconfig_location(node) (((node)->def_config & AC_DEFCFG_LOCATION) >> AC_DEFCFG_LOCATION_SHIFT)
#define defconfig_port_conn(node) (((node)->def_config & AC_DEFCFG_PORT_CONN) >> AC_DEFCFG_PORT_CONN_SHIFT)


/**********************************************************/
/*                                Azalia base0 commands                                        */
/* registers accessed via AZBAR - base0 - AZALIA registers           */
/*                                                                                                                */
/**********************************************************/

#define HDA_GCAP                        0x00    /* Global Capabilities  */
#define HDA_VMIN                        0x02    /* Minor Version */
#define HDA_VMAJ                        0x03    /* Major Version */
#define HDA_OUTPAY                      0x04    /* Output Payload Capability */
#define HDA_INPAY                       0x06    /* Input Payload Capability */
#define HDA_GCTL                        0x08    /* Global Control */
#define CRST                            BIT0    /* Controller reset */
#define UREN                            BIT8    /* Unsolicited responses */
#define HDA_WAKEEN                      0x0C    /* Wake Enable */
#define HDA_STATESTS                    0x0E    /* Wake Status */
#define HDA_GSTST                       0x10    /* Global Status */
#define HDA_ECAP                        0x14    /* Extended capabilities - mobile only */
#define HDA_OUTSTRMPAY                  0X18    /* Output stream payload capability */
#define HDA_INSTRMPAY                   0x1A    /* Input stream payload capability */

#define HDA_INTCTL                      0x20    /* Interrupt Control */
#define HDA_INT_ALL_STREAM              0xff    /* all stream interrupts mask on IOC */
#define HDA_INT_CTRL_EN                 BIT30   /* controller interrupt enable bit */
#define HDA_INT_GLOBAL_EN               BIT31   /* global interrupt enable bit */

#define HDA_INTSTS                      0x24    /* Interrupt Status */
#define REG_WALCLK                      0x30    /* Wall Clock Counter */
#define HDA_SSYNC                       0x34    /* Stream Synchronization */

#define HDA_CORBLBASE                   0x40    /* CORB Lower Base Address */
#define HDA_CORBUBASE                   0x44    /* CORB Upper Base Address */
#define HDA_CORBWP                      0x48    /* CORB Write Pointer */
#define HDA_CORBRP                      0x4A    /* CORB Read Pointer */
#define HDA_CORBCTL                     0x4C    /* CORB Control */
#define HDA_CORBSTS                     0x4D    /* CORB Status */
#define HDA_CORBSIZE                    0x4E    /* CORB Size */

#define HDA_RIRBLBASE                   0x50    /* RIRB Lower Base Address */
#define HDA_RIRBUBASE                   0x54    /* RIRB Upper Base Address */
#define HDA_RIRBWP                      0x58    /* RIRB Write Pointer */
#define HDA_RINTCNT                     0x5A    /* Response Interrupt Count */
#define HDA_RIRBCTL                     0x5C    /* RIRB Control */
#define HDA_RIRBSTS                     0x5D    /* RIRB Status */
#define HDA_RIRBSIZE                    0x5E    /* RIRB Size */

#define HDA_IC                          0x60    /* Immediate Command  */
#define HDA_IR                          0x64    /* Immediate Response */
#define HDA_IRS                         0x68    /* Immediate Command Status */
#define IRS_BUSY                        BIT0    /* immediate command busy */
#define IRS_VALID                       BIT1    /* immediate command valid */
#define HDA_DPLBASE                     0x70    /* DMA Position Lower Base Address */
#define HDA_DPUBASE                     0x74    /* DMA Position Upper Base Address */
#define HDA_DPLBASE_ENABLE              0x1     /* Enable position buffer */

#define HDA_SD_CTL                      0x0     /* stream register offsets from stream base - not used */
#define HDA_SD_STS                      0x3
#define HDA_SD_LPIB                     0x4
#define HDA_SD_CBL                      0x8
#define HDA_SD_LVI                      0xC
#define HDA_SD_FIFOW                    0xE
#define HDA_SD_FIFOSIZE                 0x10
#define HDA_SD_FORMAT                   0x12
#define HDA_SD_BDLPL                    0x18
#define HDA_SD_BDLPU                    0x1C
#define HDA_SD_LPIBA                    0x2004

/* SDCTL - Stream Descriptor Control Register bits */
#define SD_CTL_STREAM_RESET	            BIT0     /* stream reset bit */
#define SD_CTL_DMA_START                BIT1     /* stream DMA start bit */
#define SD_CTL_STREAM_TAG_MASK          (0xf << 20) /* set bits 20 - 23 of SD_CTL register */
#define SD_CTL_STREAM_TAG_SHIFT         20

/* SDSTS - Stream Descriptor Status Register bits */
#define SD_INT_COMPLETE                 BIT2   /* completion interrupt */
#define SD_INT_FIFO_ERR                 BIT3   /* FIFO error interrupt */
#define SD_INT_DESC_ERR                 BIT4   /* descriptor error interrupt */
#define RIRB_INT_MASK                   (BIT0 | BIT2)
#define SD_INT_MASK                     (SD_INT_DESC_ERR | SD_INT_FIFO_ERR | SD_INT_COMPLETE)
#define STATESTS_INT_MASK               (BIT0 | BIT1 | BIT2)

#define HDA_SDI0CTL                     0x80    /* Stream Descriptor Control */
#define HDA_SDI0STS                     0x83    /* Stream Descriptor Status */
#define HDA_SDI0LPIB                    0x84    /* Link Position in Current Buffer */
#define HDA_SDI0CBL                     0x88    /* Cyclic Buffer Length */
#define HDA_SDI0LVI                     0x8C    /* Last Valid Index */
#define HDA_SDI0FIFOW                   0x8E    /* FIFO watermark */
#define HDA_SDI0FIFOSIZE                0x90    /* FIFO Size */
#define HDA_SDI0FORMAT                  0x92    /* Format */
#define HDA_SDI0BDLPL                   0x98    /* List Pointer - Lower */
#define HDA_SDI0BDLPU                   0x9C    /* List Pointer - Upper */
#define HDA_SDI0LPIBA                   0x2084  /* Link Posiiton in Buffer n Alias */

#define HDA_SDI1CTL                     0xA0    /* Stream Descriptor Control */
#define HDA_SDI1STS                     0xA3    /* Stream Descriptor Status */
#define HDA_SDI1LPIB                    0xA4    /* Link Position in Current Buffer */
#define HDA_SDI1CBL                     0xA8    /* Cyclic Buffer Length */
#define HDA_SDI1LVI                     0xAC    /* Last Valid Index */
#define HDA_SDI1FIFOW                   0xAE    /* FIFO watermark */
#define HDA_SDI1FIFOSIZE                0xB0    /* FIFO Size */
#define HDA_SDI1FORMAT                  0xB2    /* Format */
#define HDA_SDI1BDLPL                   0xB8    /* List Pointer - Lower */
#define HDA_SDI1BDLPU                   0xBC    /* List Pointer - Upper */
#define HDA_SDI1LPIBA                   0x20A4  /* Link Posiiton in Buffer n Alias */

#define HDA_SDI2CTL                     0xC0    /* Stream Descriptor Control */
#define HDA_SDI2STS                     0xC3    /* Stream Descriptor Status */
#define HDA_SDI2LPIB                    0xC4    /* Link Position in Current Buffer */
#define HDA_SDI2CBL                     0xC8    /* Cyclic Buffer Length */
#define HDA_SDI2LVI                     0xCC    /* Last Valid Index */
#define HDA_SDI2FIFOW                   0xCE    /* FIFO watermark */
#define HDA_SDI2FIFOSIZ                 0xD0    /* FIFO Size */
#define HDA_SDI2FORMAT                  0xD2    /* Format */
#define HDA_SDI2BDLPL                   0xD8    /* List Pointer - Lower */
#define HDA_SDI2BDLPU                   0xDC    /* List Pointer - Upper */
#define HDA_SDI2LPIBA                   0x20D4  /* Link Posiiton in Buffer n Alias */

#define HDA_SDI3CTL                     0xE0    /* Stream Descriptor Control */
#define HDA_SDI3STS                     0xE3    /* Stream Descriptor Status */
#define HDA_SDI3LPIB                    0xE4    /* Link Position in Current Buffer */
#define HDA_SDI3CBL                     0xE8    /* Cyclic Buffer Length */
#define HDA_SDI3LVI                     0xEC    /* Last Valid Index */
#define HDA_SDI3FIFOW                   0xFE    /* FIFO watermark */
#define HDA_SDI3FIFOSIZE                0xF0    /* FIFO Size */
#define HDA_SDI3FORMAT                  0xF2    /* Format */
#define HDA_SDI3BDLPL                   0xF8    /* List Pointer - Lower */
#define HDA_SDI3BDLPU                   0xFC    /* List Pointer - Upper */
#define HDA_SDI3LPIBA                   0x20E4  /* Link Posiiton in Buffer n Alias */

#define HDA_SDO0CTL                     0x100   /* Stream Descriptor Control */
#define HDA_SDO0STS                     0x103   /* Stream Descriptor Status */
#define HDA_SDO0LPIB                    0x104   /* Link Position in Current Buffer */
#define HDA_SDO0CBL                     0x108   /* Cyclic Buffer Length */
#define HDA_SDO0LVI                     0x10C   /* Last Valid Index */
#define HDA_SDO0FIFOW                   0x10E   /* FIFO watermark */
#define HDA_SDO0FIFOSIZE                0x110   /* FIFO Size */
#define HDA_SDO0FORMAT                  0x112   /* Format */
#define HDA_SDO0BDLPL                   0x118   /* List Pointer - Lower */
#define HDA_SDO0BDLPU                   0x11C   /* List Pointer - Upper */
#define HDA_SDO0LPIBA                   0x2104  /* Link Posiiton in Buffer n Alias */

#define HDA_SDO1CTL                     0x120   /* Stream Descriptor Control */
#define HDA_SDO1STS                     0x123   /* Stream Descriptor Status */
#define HDA_SDO1LPIB                    0x124   /* Link Position in Current Buffer */
#define HDA_SDO1CBL                     0x128   /* Cyclic Buffer Length */
#define HDA_SDO1LVI                     0x12C   /* Last Valid Index */
#define HDA_SDO1FIFOW                   0x12E   /* FIFO watermark */
#define HDA_SDO1FIFOSIZE                0x130   /* FIFO Size */
#define HDA_SDO1FORMAT                  0x132   /* Format */
#define HDA_SDO1BDLPL                   0x138   /* List Pointer - Lower */
#define HDA_SDO1BDLPU                   0x13C   /* List Pointer - Upper */
#define HDA_SDO1LPIBA                   0x2124  /* Link Posiiton in Buffer n Alias */

#define HDA_SDO2CTL                     0x140   /* Stream Descriptor Control */
#define HDA_SDO2STS                     0x143   /* Stream Descriptor Status */
#define HDA_SDO2LPIB                    0x144   /* Link Position in Current Buffer */
#define HDA_SDO2CBL                     0x148   /* Cyclic Buffer Length */
#define HDA_SDO2LVI                     0x14C   /* Last Valid Index */
#define HDA_SDO2FIFOW                   0x14E   /* FIFO watermark */
#define HDA_SDO2FIFOSIZE                0x150   /* FIFO Size */
#define HDA_SDO2FORMAT                  0x152   /* Format */
#define HDA_SDO2BDLPL                   0x158   /* List Pointer - Lower */
#define HDA_SDO2BDLPU                   0x15C   /* List Pointer - Upper */
#define HDA_SDO2LPIBA                   0x2144  /* Link Posiiton in Buffer n Alias */

#define HDA_SDO3CTL                     0x160   /* Stream Descriptor Control */
#define HDA_SDO3STS                     0x163   /* Stream Descriptor Status */
#define HDA_SDO3LPIB                    0x164   /* Link Position in Current Buffer */
#define HDA_SDO3CBL                     0x168   /* Cyclic Buffer Length */
#define HDA_SDO3LVI                     0x16C   /* Last Valid Index */
#define HDA_SDO3FIFOW                   0x16E   /* FIFO watermark */
#define HDA_SDO3FIFOSIZE                0x170   /* FIFO Size */
#define HDA_SDO3FORMAT                  0x172   /* Format */
#define HDA_SDO3BDLPL                   0x178   /* List Pointer - Lower */
#define HDA_SDO3BDLPU                   0x17C   /* List Pointer - Upper */


/* AC97/HDA supported controller types */
#define DEVICE_INTEL                       0    /* AC97 device Intel ICH compatible */
#define DEVICE_SIS                         1    /* AC97 device SIS compatible */
#define DEVICE_INTEL_ICH4                  2    /* AC97 device Intel ICH4 compatible */
#define DEVICE_NFORCE                      3    /* AC97 device nForce compatible */
#define DEVICE_HDA_INTEL                   4    /* HDA audio device for Intel  and others */
#define DEVICE_HDA_ATI                     5
#define DEVICE_HDA_ATIHDMI                 6
#define DEVICE_HDA_NVIDIA                  7
#define DEVICE_HDA_SIS                     8
#define DEVICE_HDA_ULI                     9
#define DEVICE_HDA_VIA                     10

#define PCI_ANY_ID              ((WORD)(~0))

static AUDIO_DEVICE_LIST audio_dev_list[] = {
        // supported controllers AC97 INTEL
        { 0x8086, 0x2415, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "Intel 82801AA (ICH) integrated AC97 audio codec" },
        { 0x8086, 0x2425, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "Intel 82801AB (ICH0) integrated AC97 audio codec" },
        { 0x8086, 0x2445, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "Intel 82801BA (ICH2) integrated AC97 audio codec" },
        { 0x8086, 0x2485, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "Intel 82801CA (ICH3) integrated AC97 audio codec" },
        { 0x8086, 0x24c5, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL_ICH4, "Intel 82801DB (ICH4) integrated AC97 audio codec" },
        { 0x8086, 0x24d5, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL_ICH4, "Intel 82801EB/ER (ICH5/ICH5R) integrated AC97 audio codec" },
        { 0x8086, 0x25a6, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL_ICH4, "Intel 6300ESB integrated AC97 audio codec" },
   	{ 0x8086, 0x266e, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL_ICH4, "Intel 82801FB (ICH6) integrated AC97 audio codec" },
        { 0x8086, 0x27de, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL_ICH4, "Intel 82801GB (ICH7) integrated AC97 audio codec" },
        { 0x8086, 0x7195, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "Intel 82443MX integrated AC97 audio codec" },

        // supported controllers AC97 other (AMD/NVIDIA/SIS)
        { 0x1022, 0x7445, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "AMD 768 integrated AC97 audio codec" },
        { 0x1022, 0x746d, PCI_ANY_ID, PCI_ANY_ID, DEVICE_INTEL, "AMD 8111 integrated AC97 audio codec" },

	{ 0x10de, 0x01b1, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce integrated AC97 audio codec" },
        { 0x10de, 0x006a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce2 integrated AC97 audio codec" },
        { 0x10de, 0x008a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia CK8 (nForce compatible) integrated AC97 audio codec" },
        { 0x10de, 0x00da, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce3 integrated AC97 audio codec" },
        { 0x10de, 0x00ea, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia CK8S (nForce compatible) integrated AC97 audio codec" },
        { 0x10de, 0x0059, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce4 integrated AC97 audio codec" },
        { 0x10de, 0x026b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce MCP51 integrated AC97 audio codec" },
        { 0x10de, 0x003a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce MCP04 integrated AC97 audio codec" },
        { 0x10de, 0x006b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_NFORCE, "Nvidia nForce MCP integrated AC97 audio codec" },

        { 0x1039, 0x7012, PCI_ANY_ID, PCI_ANY_ID, DEVICE_SIS,   "SiS SI7012 integrated AC97 audio codec" },

        // supported controllers HDA INTEL
        { 0x8086, 0x2668, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801F (ICH6) integrated High Definition Audio controller" },
        { 0x8086, 0x27d8, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801G (ICH7) integrated High Definition Audio controller" },
        { 0x8086, 0x269a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel ESB2 integrated High Definition Audio controller" },
        { 0x8086, 0x284b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801H (ICH8) integrated High Definition Audio controller" },
        { 0x8086, 0x293e, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801I (ICH9) integrated High Definition Audio controller" },
        { 0x8086, 0x293f, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801I (ICH9) integrated High Definition Audio controller" },
        { 0x8086, 0x3a3e, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801J (ICH10R) integrated High Definition Audio controller" },
        { 0x8086, 0x3a6e, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 82801J (ICH10) integrated High Definition Audio controller" },
        { 0x8086, 0x3b56, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 5-series integrated High Definition Audio controller" },
        { 0x8086, 0x1c20, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 6-series integrated High Definition Audio controller" },
        { 0x8086, 0x1d20, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_INTEL, "Intel 7-series integrated High Definition Audio controller" },

        // supported controllers HDA other (ATI/NVIDIA/SIS/ULI/VIA)
        { 0x1002, 0x437b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ATI, "ATI Technologies SB450 integrated High Definition Audio controller" },
        { 0x1002, 0x4383, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ATI, "ATI Technologies SB600 integrated High Definition Audio controller" },

        { 0x1002, 0x793b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ATIHDMI, "ATI Technologies RS600 integrated High Definition Audio controller",},
        { 0x1002, 0x7919, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ATIHDMI, "ATI Technologies RS690 integrated High Definition Audio controller",},
        { 0x1002, 0x960c, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ATIHDMI, "ATI Technologies RS780 integrated High Definition Audio controller",},
        { 0x1002, 0xaa00, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ATIHDMI, "ATI Technologies R600 integrated High Definition Audio controller",},
 
        { 0x10de, 0x026c, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP51 integrated High Definition Audio controller" },
        { 0x10de, 0x0371, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP55 integrated High Definition Audio controller" },
        { 0x10de, 0x03e4, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP61a integrated High Definition Audio controller" },
        { 0x10de, 0x03f0, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP61b integrated High Definition Audio controller" },
        { 0x10de, 0x044a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP65a integrated High Definition Audio controller" },
        { 0x10de, 0x044b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP65b integrated High Definition Audio controller" },
        { 0x10de, 0x055c, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP67a integrated High Definition Audio controller" },
        { 0x10de, 0x055d, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP67b integrated High Definition Audio controller" },
        { 0x10de, 0x07fc, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP73a integrated High Definition Audio controller" },
        { 0x10de, 0x07fd, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP73b integrated High Definition Audio controller" },
        { 0x10de, 0x0774, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP77a integrated High Definition Audio controller" },
        { 0x10de, 0x0775, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP77b integrated High Definition Audio controller" },
        { 0x10de, 0x0776, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP77c integrated High Definition Audio controller" },
        { 0x10de, 0x0777, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP77d integrated High Definition Audio controller" },
        { 0x10de, 0x0ac0, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP79a integrated High Definition Audio controller" },
        { 0x10de, 0x0ac1, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP79b integrated High Definition Audio controller" },
        { 0x10de, 0x0ac2, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP79c integrated High Definition Audio controller" },
        { 0x10de, 0x0ac3, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_NVIDIA, "Nvidia nForce MCP79d integrated High Definition Audio controller" },
        
        { 0x1039, 0x7502, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_SIS, "SIS 966 integrated High Definition Audio controller" },

        { 0x10b9, 0x5461, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_ULI, "ULI M5461 integrated High Definition Audio controller" },
        
        { 0x1106, 0x3288, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA_VIA, "VIA 8251/8237 integrated High Definition Audio controller" },
        
        // null entry
        { 0x0000, 0x0000, PCI_ANY_ID, PCI_ANY_ID, 0, "" }
};

static AUDIO_STEREO_TECHNOLOGY audio_stereo_technology[] = {
        {"No 3D Stereo Enhancement"},
        {"Analog Devices Phat Stereo"},   // 1
        {"Creative Stereo Enhancement"},   // 2
        {"National Semi 3D Stereo Enhancement"},   // 3
        {"YAMAHA Ymersion"},   // 4
        {"BBE 3D Stereo Enhancement"},   // 5
        {"Crystal Semi 3D Stereo Enhancement"},   // 6
        {"Qsound QXpander"},   // 7
        {"Spatializer 3D Stereo Enhancement"},   // 8
        {"SRS 3D Stereo Enhancement"},   // 9
        {"Platform Tech 3D Stereo Enhancement"},   // 10
        {"AKM 3D Audio"},   // 11
        {"Aureal Stereo Enhancement"},   // 12
        {"Aztech 3D Enhancement"},   // 13
        {"Binaura 3D Audio Enhancement"},   // 14
        {"ESS Technology Stereo Enhancement"},   // 15
        {"Harman International VMAx"},   // 16
        {"Nvidea 3D Stereo Enhancement"},   // 17
        {"Philips Incredible Sound"},   // 18
        {"Texas Instruments 3D Stereo Enhancement"},   // 19
        {"VLSI Technology 3D Stereo Enhancement"},   // 20
        {"TriTech 3D Stereo Enhancement"},   // 21
        {"Realtek 3D Stereo Enhancement"},   // 22
        {"Samsung 3D Stereo Enhancement"},   // 23
        {"Wolfson Microelectronics 3D Enhancement"},   // 24
        {"Delta Integration 3D Enhancement"},   // 25
        {"SigmaTel 3D Enhancement"},   // 26
        {"Winbond 3D Stereo Enhancement"},   // 27
        {"Rockwell 3D Stereo Enhancement"},   // 28
        {"Unknown 3D Stereo Enhancement"},   // 29
        {"Unknown 3D Stereo Enhancement"},   // 30
        {"Unknown 3D Stereo Enhancement"},   // 31
};

// vender id, device id, string
static AUDIO_CODEC_LIST audio_codec_list[] = {
        
        // AC97 codecs
        {0x4144, 0x5303, "Analog Devices AD1819"},
        {0x4144, 0x5340, "Analog Devices AD1881"},
        {0x4144, 0x5348, "Analog Devices AD1881A"},
        {0x4144, 0x5360, "Analog Devices AD1885"},
        {0x4144, 0x5361, "Analog Devices AD1886"},
        {0x4144, 0x5362, "Analog Devices AD1887"},
        {0x4144, 0x5363, "Analog Devices AD1886A"},        
        {0x4144, 0x5370, "Analog Devices AD1980"},

        {0x414B, 0x4D00, "Asahi Kasei AK4540"},
        {0x414B, 0x4D01, "Asahi Kasei AK4542"},
        {0x414B, 0x4D02, "Asahi Kasei AK4543"},

        {0x414C, 0x4300, "ALC100"},
        {0x414C, 0x4326, "ALC100P"},
        {0x414C, 0x4710, "ALC200/200P"},
        {0x414C, 0x4720, "ALC650"},
        {0x414C, 0x4721, "ALC650D"},
        {0x414C, 0x4722, "ALC650E"},
        {0x414C, 0x4723, "ALC650F"},
        {0x414C, 0x4760, "ALC655"},
        {0x414C, 0x4780, "ALC658"},
        {0x414C, 0x4781, "ALC658D"},
        {0x414C, 0x4790, "ALC850"},
        
        {0x434D, 0x4941, "CMedia CM9738"},
        {0x434D, 0x4942, "CMedia"},
        {0x434D, 0x4961, "CMedia CM9739"},
        {0x434D, 0x4978, "CMedia CM9761 rev A"},
        {0x434D, 0x4982, "CMedia CM9761 rev B"},
        {0x434D, 0x4983, "CMedia CM9761 rev C"},

        {0x4352, 0x5900, "Cirrus Logic CS4297"},
        {0x4352, 0x5903, "Cirrus Logic CS4297"},
        {0x4352, 0x5910, "Cirrus Logic CS4297A"},
        {0x4352, 0x5913, "Cirrus Logic CS4297A rev A"},
        {0x4352, 0x5914, "Cirrus Logic CS4297A rev B"},
        {0x4352, 0x5923, "Cirrus Logic CS4298"},
        {0x4352, 0x592B, "Cirrus Logic CS4294"},
        {0x4352, 0x592D, "Cirrus Logic CS4294"},
        {0x4352, 0x5930, "Cirrus Logic CS4299"},
        {0x4352, 0x5931, "Cirrus Logic CS4299 rev A"},
        {0x4352, 0x5933, "Cirrus Logic CS4299 rev C"},
        {0x4352, 0x5934, "Cirrus Logic CS4299 rev D"},
        {0x4352, 0x5948, "Cirrus Logic CS4201"},
        {0x4352, 0x5958, "Cirrus Logic CS4205"},

        {0x4358, 0x5442, "CXT66"},

        {0x4454, 0x3031, "Diamond Technology DT0893"},

        {0x4583, 0x8308, "ESS Allegro ES1988"},

        {0x4943, 0x4511, "ICEnsemble ICE1232"},
        {0x4943, 0x4541, "VIA Vinyl"},
        {0x4943, 0x4551, "VIA Vinyl"},
        {0x4943, 0x4552, "VIA Vinyl"},
        {0x4943, 0x6010, "VIA Vinyl"},
        {0x4943, 0x6015, "VIA Vinyl"},
        {0x4943, 0x6017, "VIA Vinyl"},

        {0x4e53, 0x4331, "National Semiconductor LM4549"},

        {0x5349, 0x4c22, "Silicon Laboratory Si3036"},
        {0x5349, 0x4c23, "Silicon Laboratory Si3038"},

        {0x5452, 0x00FF, "TriTech TR?????"},
        {0x5452, 0x4102, "TriTech TR28022"},
        {0x5452, 0x4103, "TriTech TR28023"},
        {0x5452, 0x4106, "TriTech TR28026"},
        {0x5452, 0x4108, "TriTech TR28028"},
        {0x5452, 0x4123, "TriTech TR A5"},

        {0x5745, 0x4301, "Winbond 83971D"},

        {0x574D, 0x4C00, "Wolfson WM9704"},
        {0x574D, 0x4C03, "WM9703/07/08/17"},
        {0x574D, 0x4C04, "WM9704M/WM9704Q"},
        {0x574D, 0x4C05, "Wolfson WM9705/WM9710"},

        {0x594D, 0x4803, "Yamaha YMF753"},

        {0x8384, 0x7600, "SigmaTel STAC9700"},
        {0x8384, 0x7604, "SigmaTel STAC9704"},
        {0x8384, 0x7605, "SigmaTel STAC9705"},
        {0x8384, 0x7608, "SigmaTel STAC9708"},
        {0x8384, 0x7609, "SigmaTel STAC9721/23"},
        {0x8384, 0x7644, "SigmaTel STAC9744/45"},
        {0x8384, 0x7656, "SigmaTel STAC9756/57"},
        {0x8384, 0x7666, "SigmaTel STAC9750T"},
        {0x8384, 0x7684, "SigmaTel STAC9783/84"},

        // HDA codecs
        {0x10ec, 0x0260, "ALC260 Realtek High Definition Audio"},
	{0x10ec, 0x0268, "ALC268 Realtek High Definition Audio"},
	{0x10ec, 0x0660, "ALC660 Realtek High Definition Audio"},
	{0x10ec, 0x0861, "ALC861 Realtek High Definition Audio"},
	{0x10ec, 0x0862, "ALC861VD Realtek High Definition Audio"},
	{0x10ec, 0x0880, "ALC880 Realtek High Definition Audio"},
	{0x10ec, 0x0882, "ALC882 Realtek High Definition Audio"},
	{0x10ec, 0x0883, "ALC883 Realtek High Definition Audio"},
	{0x10ec, 0x0885, "ALC885 Realtek High Definition Audio"},
	{0x10ec, 0x0888, "ALC888 Realtek High Definition Audio"},

        {0x11d4, 0x1981, "AD1981HD Analog Devices High Definition Audio"},
	{0x11d4, 0x1983, "AD1983 Analog Devices High Definition Audio"},
	{0x11d4, 0x1984, "AD1984 Analog Devices High Definition Audio"},
	{0x11d4, 0x1986, "AD1986A Analog Devices High Definition Audio"},
	{0x11d4, 0x1988, "AD1988 Analog Devices High Definition Audio"},
	{0x11d4, 0x198b, "AD1988B Analog Devices High Definition Audio"},

        {0x434d, 0x4980, "CMI9880 CMedia High Definition Audio"},

        {0x8384, 0x7680, "STAC9221 Sigmatel High Definition Audio"},
	{0x8384, 0x7683, "STAC9221D Sigmatel High Definition Audio"},
	{0x8384, 0x7690, "STAC9220 Sigmatel High Definition Audio"},
	{0x8384, 0x7681, "STAC9220D Sigmatel High Definition Audio"},
	{0x8384, 0x7618, "STAC9227 Sigmatel High Definition Audio"},
	{0x8384, 0x7627, "STAC9271D Sigmatel High Definition Audio"},

        {0x14f1, 0x5045, "CXVenice Conexant High Definition Audio"},
	{0x14f1, 0x5047, "CXWaikiki Conexant High Definition Audio"},

        {0x1106, 0x1708, "VT1708 rev8 High Definition Audio"},
	{0x1106, 0x1709, "VT1708 rev9 High Definition Audio"},
	{0x1106, 0x170a, "VT1708 rev10 High Definition Audio"},
	{0x1106, 0x170b, "VT1708 rev11 High Definition Audio"},
	{0x1106, 0xe710, "VT1709 rev0 High Definition Audio"},
	{0x1106, 0xe711, "VT1709 rev1 High Definition Audio"},
	{0x1106, 0xe712, "VT1709 rev2 High Definition Audio"},
	{0x1106, 0xe713, "VT1709 rev3 High Definition Audio"},
	{0x1106, 0xe714, "VT1709 rev4 High Definition Audio"},
	{0x1106, 0xe715, "VT1709 rev5 High Definition Audio"},
	{0x1106, 0xe716, "VT1709 rev6 High Definition Audio"},
	{0x1106, 0xe717, "VT1709 rev7 High Definition Audio"},
	{0x1106, 0xe718, "VT1709 rev8 High Definition Audio"},
	{0x1106, 0xe719, "VT1709 rev9 High Definition Audio"},

        {0x0000, 0x0000, "Unknown Device"},
};
