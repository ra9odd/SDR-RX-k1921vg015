openocd -f interface/blueprog.cfg -f target/k1921vg015.cfg -c "program sdr1921.hex verify; reset; exit"
