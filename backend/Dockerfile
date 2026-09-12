FROM node:20-alpine AS build
WORKDIR /app
COPY package*.json tsconfig.json ./
RUN npm ci
COPY src ./src
RUN npm run build

FROM node:20-alpine
ENV NODE_ENV=production
WORKDIR /app
RUN addgroup -S robot && adduser -S robot -G robot
COPY package*.json ./
RUN npm ci --omit=dev
COPY --from=build /app/dist ./dist
USER robot
EXPOSE 3000
HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 CMD wget -qO- http://127.0.0.1:3000/health >/dev/null || exit 1
CMD ["node", "dist/src/server.js"]
