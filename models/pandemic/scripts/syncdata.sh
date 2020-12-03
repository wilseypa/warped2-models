#!/bin/bash

source syncdata.conf

while true; do
      rclone --quiet --checksum sync "${LOCAL_RESULTS_BASEDIR_PATH}/${LOCAL_SIMRESULTS_DIRNAME}" \
             "${REMOTE_NAME}:${REMOTE_RESULTS_BASEDIR_PATH}/${REMOTE_SIMRESULTS_DIRNAME}" && \
          rclone --quiet --checksum sync "${LOCAL_RESULTS_BASEDIR_PATH}/${LOCAL_LOGS_DIRNAME}" \
                 "${REMOTE_NAME}:${REMOTE_RESULTS_BASEDIR_PATH}/${REMOTE_LOGS_DIRNAME}" && \
          rclone --quiet --checksum sync "${LOCAL_RESULTS_BASEDIR_PATH}/${LOCAL_PLOTS_RELATIVE_PATH}" \
                 "${REMOTE_NAME}:${REMOTE_RESULTS_BASEDIR_PATH}/${REMOTE_PLOTS_DIRNAME}" && \
          [[ $? == 0 ]] && echo "synced successfully" || echo "error"

      echo "sleeping ..."
      sleep 200

done


