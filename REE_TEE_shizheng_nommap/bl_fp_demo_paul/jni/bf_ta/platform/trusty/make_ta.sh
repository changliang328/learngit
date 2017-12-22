rm -rf build

make M="app/demo/bf_ta:TA" -B
echo -e "make M="app/demo/bf_ta:TA" -B"

cp build/user_tasks/app/demo/bf_ta/*.elf  out_ta/
echo -e "cp  *.elf  to  out_ta/   done."

sh  mk_ta_tosimg.sh 
echo -e "----------------------------------------------------------------"
echo -e "----------------------------------------------------------------"
echo  -e "Gen tos.bin to     out_ta/tos.bin"
echo -e "----------------------------------------------------------------"
echo -e "----------------------------------------------------------------"
