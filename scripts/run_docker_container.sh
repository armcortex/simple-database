docker run \
  -it \
  --rm \
  --mount type=bind,source="../$(pwd)",target=/app \
  my-simple-db