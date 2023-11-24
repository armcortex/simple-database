docker run \
  -it \
  --rm \
  --mount type=bind,source="$(pwd)",target=/app \
  --mount type=bind,source=/etc/localtime,target=/etc/localtime,readonly \
  --mount type=bind,source=/etc/timezone,target=/etc/timezone,readonly \
  my-simple-db
