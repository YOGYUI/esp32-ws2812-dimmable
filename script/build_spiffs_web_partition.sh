#! /usr/sh
# build_spiffs_web_partition.sh
# author: seung hee, lee
# date: 2023-02-15
# purpose: build web resource as partition binary file(.bin)
# reference: components/spiffs/project_include.cmake::spiffs_create_partition_image
# reference: components/partition_table/project_include.cmake::partition_table_get_partition_info

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
PARTITION_CSV_PATH=${project_path}/partitions.csv
RESOURCE_DIR=${project_path}/main/web/dist
BIN_OUT_PATH=${project_path}/main/web/out/${PARTITION_LABEL}.bin

PARTITION_TABLE_OFFSET=0xC000   # defined in sdkconfig (CONFIG_PARTITION_TABLE_OFFSET)
SPIFFS_PAGE_SIZE=256            # defined in sdkconfig (CONFIG_SPIFFS_PAGE_SIZE)
SPIFFS_OBJ_NAME_LEN=64          # defined in sdkconfig (CONFIG_SPIFFS_OBJ_NAME_LEN)
SPIFFS_META_LENGTH=4            # defined in sdkconfig (CONFIG_SPIFFS_META_LENGTH)

# 1) get partition size and offset from partition csv file (parttool.py)
PARTITION_SIZE=$(${python} ${esp_idf_path}/components/partition_table/parttool.py \
                    --partition-table-offset ${PARTITION_TABLE_OFFSET} \
                    --partition-table-file ${PARTITION_CSV_PATH} \
                    get_partition_info --partition-name ${PARTITION_LABEL} --info size)
PARTITION_OFFSET=$(${python} ${esp_idf_path}/components/partition_table/parttool.py \
                    --partition-table-offset ${PARTITION_TABLE_OFFSET} \
                    --partition-table-file ${PARTITION_CSV_PATH} \
                    get_partition_info --partition-name ${PARTITION_LABEL} --info offset)
echo "partition (${PARTITION_LABEL}): offset=${PARTITION_OFFSET}, size=${PARTITION_SIZE}"

# 2) create spiffs binary file (spiffsgen.py)
BIN_OUT_DIR=$(dirname ${BIN_OUT_PATH})
[ ! -d "$BIN_OUT_DIR" ] && mkdir -p "$BIN_OUT_DIR"

$(${python} ${esp_idf_path}/components/spiffs/spiffsgen.py \
  ${PARTITION_SIZE} ${RESOURCE_DIR} ${BIN_OUT_PATH} \
  --page-size ${SPIFFS_PAGE_SIZE} --obj-name-len ${SPIFFS_OBJ_NAME_LEN} \
  --meta-len ${SPIFFS_META_LENGTH} \
  --use-magic \
  --use-magic-len)
echo "spiffs partition binary file generated >> ${BIN_OUT_PATH}"
