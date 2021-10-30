local fib
local main
fib = function(i0_n)
    local i_0
    local i_1
    local i_2
    local i_3
    local i_4
    local i_5
    if i0_n >= 2 then do goto l1 end end
::l2::
    do return i0_n end
    do goto l3 end
::l1::
    i_2 = i0_n - 2
    i_1 = fib(i_2);
    i_4 = i0_n - 1
    i_3 = fib(i_4);
    i_5 = i_1 + i_3
    do return i_5 end
::l3::
    do return 0 end
end
main = function()
    local i_0
    local i_1
    i_1 = fib(35);
    i_0 = print(i_1);
    do return 0 end
end
main(0, 0)