        lw 0 7 32767
        sw 7 0 -32768
        beq 0 7 -32768
        add 0 7 0
        halt
max     .fill 2147483647
min     .fill -2147483648


