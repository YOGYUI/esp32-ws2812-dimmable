#! /usr/sh
# flash_spiffs_web_partition.sh
# author: seung hee, lee
# date: 2023-02-15
# purpose: upload spiffs binary partition to esp32 flash
# reference: components/spiffs/project_include.cmake::spiffs_create_partition_image

port=/dev/ttyUSB0
port_arg=0
for arg in "$@"
do
    if [[ "$arg" == "-p" ]]; then
        port_arg=1
    else
        if [[ $port_arg == 1 ]]; then
            port=$arg
            port_arg=0
        fi
    fi
done

cur_path=${PWD}
if [[ "$OSTYPE" == "darwin"* ]]; then
    project_path=$(dirname $(dirname $(realpath $0)))
else 
    project_path=$(dirname $(dirname $(realpath $BASH_SOURCE)))
fi
esp_idf_path=${project_path}/sdk/esp-idf

# prepare environment (python)
if [ -z "$IDF_PATH" ]; then
  source ${esp_idf_path}/export.sh
fi

# set variables
PARTITION_LABEL=web
PARTITION_TABLE_OFFSET=0xC000   # defined in sdkconfig (CONFIG_PARTITION_TABLE_OFFSET)
PARTITION_CSV_PATH=${project_path}/partitions.csv
BIN_OUT_PATH=${project_path}/main/web/out/${PARTITION_LABEL}.bin

# get offset of partition label
PARTITION_OFFSET=$(${python} ${esp_idf_path}/components/partition_table/parttool.py \
                    --partition-table-offset ${PARTITION_TABLE_OFFSET} \
                    --partition-table-file ${PARTITION_CSV_PATH} \
                    get_partition_info --partition-name ${PARTITION_LABEL} --info offset)

# flash manufacturing binary
esptool.py -p ${port} -b 921600 write_flash ${PARTITION_OFFSET} ${BIN_OUT_PATH}
