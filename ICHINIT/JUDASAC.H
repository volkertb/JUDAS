
/*
 * Internal header file: simplified definitions (used by AC97 code)
 * by Piotr Ulaszewski (PETERS)
 * August 2007 rev
 */

#define FALSE          0
#define TRUE           1
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


// pci static defines
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

#define CTL_BASE   0		/* addressing controller regs */
#define MIXER_BASE 1		/* addressing mixer regs */


/*
 * Internal header file: Intel ICH AC97 and compatibles.
 * (AC97 pci device structure and defines partially based on MAME AC97 support header)
 *
 */

typedef struct {
        WORD vender_id;
        WORD device_id;
        WORD sub_vender_id;
        WORD sub_device_id;
        int  type;
        char *string;
} AC97_DEVICE_LIST;

typedef struct {
        char *string;
} AC97_STEREO_TECHNOLOGY;

typedef struct {
        WORD vender_id1;
        WORD vender_id2;
        char *string;
} AC97_CODEC_LIST;


/*
 * AC97 PCI device structure for device access
 */
#pragma pack(push,1)
 typedef struct {
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
        int mem_mode;               // 0 for IO access, 1 for memory access
        int hda_mode;               // 0 for AC97 mode, 1 for HDA mode
        WORD codec_id1;
        WORD codec_id2;
        char device_name[128];      // controller name
        char codec_name[128];       // codec name
} AC97_PCI_DEV;
#pragma pack(pop)


/* registers accessed via NAMBAR - base0 - Audio Mixer registers */

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


/* AC97 supported controller types */
#define DEVICE_INTEL                       0    /* AC97 device Intel ICH compatible */
#define DEVICE_SIS                         1    /* AC97 device SIS compatible */
#define DEVICE_INTEL_ICH4                  2    /* AC97 device Intel ICH4 compatible */
#define DEVICE_NFORCE                      3    /* AC97 device nForce compatible */
#define DEVICE_HDA                         4    /* HDA audio device */

#define PCI_ANY_ID              ((WORD)(~0))

static AC97_DEVICE_LIST ac97_dev_list[] = {
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
        { 0x8086, 0x2668, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 82801FB (ICH6) integrated High Definition Audio controller" },
        { 0x8086, 0x27d8, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 82801G (ICH7) integrated High Definition Audio controller" },
        { 0x8086, 0x269a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel ESB2 integrated High Definition Audio controller" },
        { 0x8086, 0x284b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 82801H (ICH8) integrated High Definition Audio controller" },
        { 0x8086, 0x293f, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 82801I (ICH9) integrated High Definition Audio controller" },
        { 0x8086, 0x3a6e, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 82801J (ICH10) integrated High Definition Audio controller" },
        { 0x8086, 0x3b56, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 5-series integrated High Definition Audio controller" },
        { 0x8086, 0x1c20, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 6-series integrated High Definition Audio controller" },
        { 0x8086, 0x1d20, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Intel 7-series integrated High Definition Audio controller" },

        // supported controllers HDA other (ATI/NVIDIA/SIS/ULI/VIA)
        { 0x1002, 0x437b, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "ATI Technologies SB450 integrated High Definition Audio controller" },
        { 0x1002, 0x4383, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "ATI Technologies SB600 integrated High Definition Audio controller" },

        { 0x10de, 0x026c, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Nvidia nForce MCP51 integrated High Definition Audio controller" },
        { 0x10de, 0x0371, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Nvidia nForce MCP55 integrated High Definition Audio controller" },
        { 0x10de, 0x03e4, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Nvidia nForce MCP61 integrated High Definition Audio controller" },
        { 0x10de, 0x03f0, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Nvidia nForce MCP61b integrated High Definition Audio controller" },
        { 0x10de, 0x044a, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Nvidia nForce MCP65 integrated High Definition Audio controller" },
        { 0x10de, 0x055c, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "Nvidia nForce MCP67 integrated High Definition Audio controller" },

        { 0x1039, 0x7502, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "SIS Technologies integrated High Definition Audio controller" },

        { 0x10b9, 0x5461, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "ULI integrated High Definition Audio controller" },

        { 0x1106, 0x3288, PCI_ANY_ID, PCI_ANY_ID, DEVICE_HDA, "VIA Technologies integrated High Definition Audio controller" },

        // null entry
        { 0x0000, 0x0000, PCI_ANY_ID, PCI_ANY_ID, 0, "" }
};

static AC97_STEREO_TECHNOLOGY ac97_stereo_technology[] = {
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

static AC97_CODEC_LIST ac97_codec_list[] = {
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

        {0x0000, 0x0000, "Unknown Device"},
};
