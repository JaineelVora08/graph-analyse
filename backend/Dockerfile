FROM gcc:13-bookworm AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && cmake --build build --config Release --target graph_engine

FROM debian:bookworm-slim

WORKDIR /app
COPY --from=build /app/build/graph_engine /app/graph_engine

EXPOSE 8080
CMD ["./graph_engine"]
