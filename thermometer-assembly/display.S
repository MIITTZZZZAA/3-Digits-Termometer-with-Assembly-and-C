.section .init1
init_stack:
    ; setam stack pointer-ul la capatul RAM-ului (0x25F)
    ; fara asta, apelurile de functie si variabilele locale pot da erori
    ldi r16, lo8(0x25F)     ; incarcam byte-ul low din adresa 0x25F in r16
    out 0x3D, r16           ; SPL (Stack Pointer Low) <- r16
    ldi r16, hi8(0x25F)     ; incarcam byte-ul high din adresa 0x25F in r16
    out 0x3E, r16           ; SPH (Stack Pointer High) <- r16

.section .text
.global afiseaza_digit1
.global afiseaza_digit2
.global afiseaza_digit3
.global digit1
.global digit2
.global digit3
.global decode
.global display
.global long_delay
.extern digits              ; digits este definit in C si partajat

.equ PORTA, 0x1B            ; adresa registrului PORTA (segmente A-G + DP)
.equ PORTB, 0x18            ; adresa registrului PORTB (selectie cifra)

; afiseaza prima cifra de pe afisaj (stanga)
afiseaza_digit1:
    lds r20, digits         ; incarcam digits[0] in r20
    rcall digit1            ; apelam functia care afiseaza cifra 1
    ret

; afiseaza a doua cifra (mijloc)
afiseaza_digit2:
    lds r20, digits+1       ; incarcam digits[1] in r20
    rcall digit2
    ret

; afiseaza a treia cifra (dreapta)
afiseaza_digit3:
    lds r20, digits+2       ; incarcam digits[2] in r20
    rcall digit3
    ret

; functie care afiseaza cifra 1 (digit1)
digit1:
    ldi r16, 0x00
    out PORTB, r16          ; dezactiveaza toate cifrele (PB3–PB5)
    rcall decode            ; traduce cifra din r20 in cod de segmente -> r16
    ldi r17, 0b00001000     ; activeaza cifra 1 (PB3)
    rcall display           ; trimite r16 (segmente) + r17 (digit activ)
    ret

; functie care afiseaza cifra 2 si punctul zecimal
digit2:
    ldi r16, 0x00
    out PORTB, r16          ; stingem toate cifrele inainte
    rcall decode
    andi r16, 0b11111110    ; punem DP (punctul zecimal) la 0 (adica il aprindem)
    ldi r17, 0b00010000     ; activam cifra 2 (PB4)
    rcall display
    ret

; functie care afiseaza cifra 3 (dreapta)
digit3:
    ldi r16, 0x00
    out PORTB, r16
    rcall decode
    ldi r17, 0b00100000     ; activam cifra 3 (PB5)
    rcall display
    ret

; afiseaza cifra activa pe display
; r16 contine codul segmentelor (A-G + DP)
; r17 contine ce cifra activam (PB3, PB4 sau PB5)
display:
    out PORTA, r16          ; scriem codul pentru segmente (PA0–PA7)
    out PORTB, r17          ; activam cifra corespunzatoare
    rcall long_delay        ; pastram cifra aprinsa putin timp
    ldi r16, 0
    out PORTB, r16          ; stingem cifra (altfel se suprapune cu urmatoarea)
    ret

; decodeaza o cifra (0–9) in cod pentru 7 segmente (anod comun)
; output: r16 = codul segmentelor
decode:
    cpi r20, 0              ; comparam cifra primita cu 0
    ldi r16, 0x03           ; cod pt cifra 0 (segmente aprinse = 0)
    breq done
    cpi r20, 1
    ldi r16, 0x9F           ; cod pt cifra 1 (doar B si C aprinse)
    breq done
    cpi r20, 2
    ldi r16, 0x25
    breq done
    cpi r20, 3
    ldi r16, 0x0D
    breq done
    cpi r20, 4
    ldi r16, 0x99
    breq done
    cpi r20, 5
    ldi r16, 0x49
    breq done
    cpi r20, 6
    ldi r16, 0x41
    breq done
    cpi r20, 7
    ldi r16, 0x1B
    breq done
    cpi r20, 8
    ldi r16, 0x01
    breq done
    cpi r20, 9
    ldi r16, 0x09
done:
    ret

; delay mare, tine cifra afisata vizibila
; practic o pauza realizata prin bucle nested
long_delay:
    ldi r18, 200            ; r18 = bucla externa
outer:
    ldi r19, 250            ; r19 = bucla interna
inner:
    nop                     ; nu face nimic, doar consuma timp
    dec r19
    brne inner              ; repeta bucla interna pana r19 = 0
    dec r18
    brne outer              ; repeta bucla externa pana r18 = 0
    ret
