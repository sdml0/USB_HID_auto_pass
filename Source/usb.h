#ifndef USB_H
#define USB_H
#include <stdint.h>

#define STM32ENDPOINTS									8
#define USB_PMAADDR										((uint32_t)0x40006000)		// The size of the Packet Memory is 512 bytes, structured as 256 words by 16 bits
#define USB_BTABLE										((usb_btable_entity_t*)USB_PMAADDR)
#define USB_BTABLE_OFFSET								(sizeof(usb_btable_entity_t) * STM32ENDPOINTS)			// Can be smaller if not all endpoints used....
#define USB_BTABLE_SMALL_BLOCK_SIZE					2
#define USB_BTABLE_LARGE_BLOCK_SIZE					32
#define USB_BTABLE_SMALL_BLOCK_SIZE_LIMIT			62
#define USB_BTABLE_SIZE									512											// first <=64 bytes of USB_BTABLE are registers
#define USB_COUNT_RX_COUNT								((uint16_t)0x03FF)
#define USB_COUNT_RX_BLSIZE							((uint16_t)0x8000)

#define USB_BASE											((uint32_t)0x40005C00)
#define USB													((USB_TypeDef *) USB_BASE)
#define EP_REG(ep_num)									*((volatile uint16_t*)USB_BASE + ((ep_num) << 1))

/* bit positions */
#define USB_RX_NUM_BLOCK_Pos							(10U)
#define USB_EP_CTR_RX_Pos                       (15U)                          
#define USB_EP_CTR_RX_Msk                       (0x1U << USB_EP_CTR_RX_Pos)    //!< 0x00008000
#define USB_EP_CTR_RX                           USB_EP_CTR_RX_Msk              //!< EndPoint Correct TRansfer RX
#define USB_EP_DTOG_RX_Pos                      (14U)                          
#define USB_EP_DTOG_RX_Msk                      (0x1U << USB_EP_DTOG_RX_Pos)   //!< 0x00004000
#define USB_EP_DTOG_RX                          USB_EP_DTOG_RX_Msk             //!< EndPoint Data TOGGLE RX
#define USB_EPRX_STAT_Pos                       (12U)                          
#define USB_EPRX_STAT_Msk                       (0x3U << USB_EPRX_STAT_Pos)    //!< 0x00003000
#define USB_EPRX_STAT                           USB_EPRX_STAT_Msk              //!< EndPoint RX STATus bit field
#define USB_EP_SETUP_Pos                        (11U)                          
#define USB_EP_SETUP_Msk                        (0x1U << USB_EP_SETUP_Pos)     //!< 0x00000800
#define USB_EP_SETUP                            USB_EP_SETUP_Msk               //!< EndPoint SETUP
#define USB_EP_T_FIELD_Pos                      (9U)                           
#define USB_EP_T_FIELD_Msk                      (0x3U << USB_EP_T_FIELD_Pos)   //!< 0x00000600
#define USB_EP_T_FIELD                          USB_EP_T_FIELD_Msk             //!< EndPoint TYPE
#define USB_EP_KIND_Pos                         (8U)                           
#define USB_EP_KIND_Msk                         (0x1U << USB_EP_KIND_Pos)      //!< 0x00000100
#define USB_EP_KIND                             USB_EP_KIND_Msk                //!< EndPoint KIND
#define USB_EP_CTR_TX_Pos                       (7U)                           
#define USB_EP_CTR_TX_Msk                       (0x1U << USB_EP_CTR_TX_Pos)    //!< 0x00000080
#define USB_EP_CTR_TX                           USB_EP_CTR_TX_Msk              //!< EndPoint Correct TRansfer TX
#define USB_EP_DTOG_TX_Pos                      (6U)                           
#define USB_EP_DTOG_TX_Msk                      (0x1U << USB_EP_DTOG_TX_Pos)   //!< 0x00000040
#define USB_EP_DTOG_TX                          USB_EP_DTOG_TX_Msk             //!< EndPoint Data TOGGLE TX
#define USB_EPTX_STAT_Pos                       (4U)                           
#define USB_EPTX_STAT_Msk                       (0x3U << USB_EPTX_STAT_Pos)    //!< 0x00000030
#define USB_EPTX_STAT                           USB_EPTX_STAT_Msk              //!< EndPoint TX STATus bit field
#define USB_EPADDR_FIELD_Pos                    (0U)                           
#define USB_EPADDR_FIELD_Msk                    (0xFU << USB_EPADDR_FIELD_Pos) //!< 0x0000000F
#define USB_EPADDR_FIELD                        USB_EPADDR_FIELD_Msk           //!< EndPoint ADDRess FIELD 
/*!< EP_TYPE[1:0] EndPoint TYPE */
#define USB_EP_TYPE_MASK_Pos                    (9U)                           
#define USB_EP_TYPE_MASK_Msk                    (0x3U << USB_EP_TYPE_MASK_Pos) /*!< 0x00000600 */
#define USB_EP_TYPE_MASK                        USB_EP_TYPE_MASK_Msk           /*!< EndPoint TYPE Mask */
#define USB_EP_BULK                             0x00000000U                    /*!< EndPoint BULK */
#define USB_EP_CONTROL                          0x00000200U                    /*!< EndPoint CONTROL */
#define USB_EP_ISOCHRONOUS                      0x00000400U                    /*!< EndPoint ISOCHRONOUS */
#define USB_EP_INTERRUPT                        0x00000600U                    /*!< EndPoint INTERRUPT */
#define USB_EP_T_MASK                          (~USB_EP_T_FIELD & USB_EPREG_MASK)
#define USB_EPKIND_MASK                        (~USB_EP_KIND & USB_EPREG_MASK)  /*!< EP_KIND EndPoint KIND */
/*!< STAT_TX[1:0] STATus for TX transfer */
#define USB_EP_TX_DIS                           0x00000000U                    /*!< EndPoint TX DISabled */
#define USB_EP_TX_STALL                         0x00000010U                    /*!< EndPoint TX STALLed */
#define USB_EP_TX_NAK                           0x00000020U                    /*!< EndPoint TX NAKed */
#define USB_EP_TX_VALID                         0x00000030U                    /*!< EndPoint TX VALID */
#define USB_EPTX_DTOG1                          0x00000010U                    /*!< EndPoint TX Data TOGgle bit1 */
#define USB_EPTX_DTOG2                          0x00000020U                    /*!< EndPoint TX Data TOGgle bit2 */
#define USB_EPTX_DTOGMASK								(USB_EPTX_STAT|USB_EPREG_MASK)
/*!< STAT_RX[1:0] STATus for RX transfer */
#define USB_EP_RX_DIS                           0x00000000U                    /*!< EndPoint RX DISabled */
#define USB_EP_RX_STALL                         0x00001000U                    /*!< EndPoint RX STALLed */
#define USB_EP_RX_NAK                           0x00002000U                    /*!< EndPoint RX NAKed */
#define USB_EP_RX_VALID                         0x00003000U                    /*!< EndPoint RX VALID */
#define USB_EPRX_DTOG1                          0x00001000U                    /*!< EndPoint RX Data TOGgle bit1 */
#define USB_EPRX_DTOG2                          0x00002000U                    /*!< EndPoint RX Data TOGgle bit1 */
/* EndPoint REGister MASK (no toggle fields) */
#define USB_EPREG_MASK                      		(USB_EP_CTR_RX|USB_EP_SETUP|USB_EP_T_FIELD|USB_EP_KIND|USB_EP_CTR_TX|USB_EPADDR_FIELD)

typedef struct {
    volatile uint32_t	tx_offset;
    volatile uint32_t	tx_count;
    volatile uint32_t	rx_offset;
    volatile uint32_t	rx_count;
} usb_btable_entity_t;

typedef struct {
    volatile uint32_t EPnR[STM32ENDPOINTS];
    volatile uint32_t RESERVED[STM32ENDPOINTS];
    volatile uint32_t CNTR;
    volatile uint32_t ISTR;
    volatile uint32_t FNR;
    volatile uint32_t DADDR;
    volatile uint32_t BTABLE;
} USB_TypeDef;

#endif /* USB_H */
