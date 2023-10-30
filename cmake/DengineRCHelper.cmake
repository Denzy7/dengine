function (dengine_rc_compile in out)
    set(rc_args ${ARGV2})
    add_custom_command(
        OUTPUT ${out}
        COMMAND dengine-rc ${rc_args} ${in} ${out}
        DEPENDS ${in}
        VERBATIM
    )
endfunction()
