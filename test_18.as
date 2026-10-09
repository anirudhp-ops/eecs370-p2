        lw 0 1 data
        beq 1 1 End
data    .fill 5
        add 1 1 1
loop    beq 0 0 loop
        b loop
End     halt
Ptr     .fill data
        .fill End
