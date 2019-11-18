#-----------------------------------------------------
# Get character from SCC2691 UART
#-----------------------------------------------------
uart_getch:
                btst    #B_STATUS_RXRDY, (REG_SR).w
                beq.s   uart_getch
                move.b  (REG_RHR).w, d0
                rts



#-----------------------------------------------------
# Determine if data is ready to be read
# CF=1 : Data in RHR is ready to be read
# CF=0 : No data in RHR
#-----------------------------------------------------
uart_is_rxdata_ready:
                clc
                btst    #B_STATUS_RXRDY, (REG_SR).w
                beq.s   .done
                sec
.done:          rts


#-----------------------------------------------------
# Loopback test
#-----------------------------------------------------
uart_internal_loopback_test:
                
                move.b  #0x1A, (REG_CR).w
                move.b  #0x13, (REG_MR).w
                move.b  #0x87, (REG_MR).w
                move.b  #0x66, (REG_CSR).w
                move.b  #0x20, (REG_CR).w
                move.b  #0x30, (REG_CR).w
                move.b  #0x45, (REG_CR).w

                move.b  #0xAA, (REG_POST).w

                # send 256 chars

                moveq   #0, d0
next_char:
                # Send TX character
                move.b  d0, (REG_THR).w

                # Clear timeout
                moveq   #0, d6

                # Poll RXRDY status
poll_rxrdy:     move.b  (REG_SR).w, d7
                andi.b  #0x01, d7
                cmpi.b  #0x01, d7
                beq.s   poll_done

                # Bump wait count
                addq.l  #1, d6
                cmpi.l  #0x40000, d6
                bne.s   poll_rxrdy

                # Error: Timeout
                move.b  #0xBB, (REG_POST).w
                stop    #0x2700
poll_done:                
                # Get RX character and compare to TX character
                move.b  (REG_RHR).w, d1
                cmp.b   d1, d0
                bne.s   char_error

                # Test next character
                addq.b  #1, d0
                bne.s   next_char

                # Status: Test OK
                move.b  #0xC3, (REG_POST).w
                stop    #0x2700

char_error:                
                # Status: RX'd character mismatch
                move.b  #0x55, (REG_POST).w
                stop    #0x2700
                rts                
