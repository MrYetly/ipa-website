#!/bin/bash
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (e.g. sudo ./systemd/install.sh)"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "Installing systemd services..."

# Copy service files and fix WorkingDirectory to match actual project location
sed "s|WorkingDirectory=.*|WorkingDirectory=$PROJECT_DIR|" "$SCRIPT_DIR/ipa-website.service" > /etc/systemd/system/ipa-website.service
cp "$SCRIPT_DIR/ipa-tunnel.service" /etc/systemd/system/ipa-tunnel.service

# Create website env file if missing
if [ ! -f /etc/default/ipa-website ]; then
    cat <<EOF > /etc/default/ipa-website
# Optional overrides for ipa-website
PORT=1362
# STATIC_DIR=/home/deploy/dev/ipa-website/static/
EOF
    chmod 644 /etc/default/ipa-website
    echo ""
    echo "Created /etc/default/ipa-website."
else
    echo "/etc/default/ipa-website already exists."
fi

# Create tunnel env file if missing
if [ ! -f /etc/default/ipa-tunnel ]; then
    echo 'TUNNEL_TOKEN=REPLACE_WITH_YOUR_TOKEN' > /etc/default/ipa-tunnel
    chmod 600 /etc/default/ipa-tunnel
    echo ""
    echo "Created /etc/default/ipa-tunnel."
    echo "EDIT THIS FILE: add your Cloudflare tunnel token before starting the service."
else
    echo "/etc/default/ipa-tunnel already exists."
fi

systemctl daemon-reload

echo ""
echo "Done. Next steps:"
echo "1. Edit /etc/default/ipa-tunnel and set your TUNNEL_TOKEN"
echo "2. systemctl enable --now ipa-website.service"
echo "3. systemctl enable --now ipa-tunnel.service"
echo ""
echo "Check status with:"
echo "  systemctl status ipa-website.service"
echo "  systemctl status ipa-tunnel.service"
