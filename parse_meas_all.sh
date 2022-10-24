
for ID in `seq 0 3`;
do
	python $GAP_SDK_HOME/utils/power_meas_utils/log_to_csv.py --model_name efficientnet_lite_${ID} --suffix ne16 --freq 370 --voltage 800 --log_dir log_meas/ --out_file meas_all.csv --model_id ${ID}
	python $GAP_SDK_HOME/utils/power_meas_utils/log_to_csv.py --model_name efficientnet_lite_${ID} --suffix ne16 --freq 240 --voltage 650 --log_dir log_meas/ --out_file meas_all.csv --model_id ${ID}
done