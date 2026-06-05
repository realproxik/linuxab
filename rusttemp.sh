# Single file
rustfmt --config-path .rustfmt.toml kernel/rust/kernel.rs

# Check mode
rustfmt --check --config-path .rustfmt.toml **/*.rs

# Recursive via cargo
cargo fmt -- --config-path .rustfmt.toml