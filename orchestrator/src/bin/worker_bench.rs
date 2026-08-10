use fileman_orchestrator::worker_pool::AtomicBatchPool;
use std::hint::black_box;
use std::time::{Duration, Instant};

const BATCHES: &[usize] = &[1, 32, 256, 4_096, 65_536];

fn transform(value: &mut u64) {
    let mut current = *value;
    for _ in 0..32 {
        current = current.wrapping_add(0x9e37_79b9_7f4a_7c15);
        current = (current ^ (current >> 30)).wrapping_mul(0xbf58_476d_1ce4_e5b9);
        current = (current ^ (current >> 27)).wrapping_mul(0x94d0_49bb_1331_11eb);
        current ^= current >> 31;
    }
    *value = current;
}

fn percentile(sorted: &[Duration], numerator: usize, denominator: usize) -> u128 {
    let index = (sorted.len() - 1) * numerator / denominator;
    sorted[index].as_nanos()
}

fn report(implementation: &str, threads: usize, batch: usize, durations: &mut [Duration]) {
    durations.sort_unstable();
    let total: u128 = durations.iter().map(Duration::as_nanos).sum();
    println!(
        "{{\"implementation\":\"{implementation}\",\"threads\":{threads},\"batch\":{batch},\"samples\":{},\"mean_ns\":{},\"p50_ns\":{},\"p95_ns\":{},\"p99_ns\":{},\"max_ns\":{}}}",
        durations.len(),
        total / durations.len() as u128,
        percentile(durations, 50, 100),
        percentile(durations, 95, 100),
        percentile(durations, 99, 100),
        durations.last().expect("nonempty durations").as_nanos()
    );
}

fn main() {
    let threads = std::env::args().nth(1).map_or_else(
        || std::thread::available_parallelism().map_or(1, usize::from),
        |value| value.parse().expect("thread count"),
    );
    let pool = AtomicBatchPool::new(threads).expect("atomic batch pool");
    for &batch in BATCHES {
        let samples = if batch >= 65_536 { 40 } else { 200 };
        let mut parallel_values = vec![1_u64; batch];
        let mut serial_values = parallel_values.clone();
        for _ in 0..20 {
            pool.for_each(&mut parallel_values, transform)
                .expect("warmup");
        }

        let mut parallel = Vec::with_capacity(samples);
        for _ in 0..samples {
            let started = Instant::now();
            pool.for_each(&mut parallel_values, transform)
                .expect("batch");
            parallel.push(started.elapsed());
        }
        black_box(&parallel_values);
        report("rust_atomic_batch", threads, batch, &mut parallel);

        let mut serial = Vec::with_capacity(samples);
        for _ in 0..samples {
            let started = Instant::now();
            serial_values.iter_mut().for_each(transform);
            serial.push(started.elapsed());
        }
        black_box(&serial_values);
        report("rust_serial", 1, batch, &mut serial);
    }
}
