//! Fixed-worker synchronous atomic batch executor.
//!
//! This is an available internal compute primitive, not the local-session host
//! and not a plugin capability. One caller publishes one borrowed batch, the
//! workers claim each index exactly once, and the caller returns only after all
//! task writes happen-before its completion acquire. Batches are serialized.
//!
//! The implementation deliberately contains one small unsafe erasure seam so
//! persistent workers can execute a synchronously borrowed slice without a
//! per-task allocation. The public API remains safe. The complete invariants
//! and memory-order argument live in `formal/ATOMIC_BATCH_POOL.md`.

#![allow(unsafe_code)]

use std::fmt;
use std::io;
use std::marker::PhantomData;
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::ptr;
use std::sync::atomic::{AtomicPtr, AtomicU32, AtomicU64, Ordering};
use std::sync::{Arc, Condvar, Mutex, MutexGuard};
use std::thread::{self, JoinHandle};

const MAXIMUM_TASKS: usize = u32::MAX as usize;
const CLAIMS_PER_WORKER_TARGET: u32 = 8;
const MAXIMUM_CLAIM_SIZE: u32 = 256;
const MINIMUM_TASKS_PER_WORKER: usize = 64;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BatchRunError {
    TooManyTasks,
    ZeroSizedTask,
    TaskPanicked { count: u32 },
}

impl fmt::Display for BatchRunError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::TooManyTasks => write!(formatter, "atomic batch exceeds the u32 ticket domain"),
            Self::ZeroSizedTask => {
                write!(formatter, "atomic batch does not admit zero-sized tasks")
            }
            Self::TaskPanicked { count } => {
                write!(formatter, "{count} atomic batch task(s) panicked")
            }
        }
    }
}

impl std::error::Error for BatchRunError {}

struct SleepState {
    terminate: bool,
}

struct Core {
    sleep: Mutex<SleepState>,
    run: Mutex<()>,
    work: Condvar,
    done: Condvar,
    batch: AtomicPtr<ErasedBatch>,
    remaining: AtomicU64,
    pending: AtomicU32,
    claim_size: AtomicU32,
}

struct ErasedBatch {
    tasks: *mut (),
    function: *const (),
    call: unsafe fn(*mut (), *const (), usize, &AtomicU32),
    panics: AtomicU32,
    _borrowed: PhantomData<*mut ()>,
}

/// A fixed set of persistent workers for synchronous homogeneous batches.
///
/// `AtomicBatchPool` owns no authority and starts no work by itself. Calls to
/// [`for_each`](Self::for_each) are serialized, may originate concurrently,
/// and borrow their task storage only until the synchronous call returns.
pub struct AtomicBatchPool {
    core: Arc<Core>,
    workers: Mutex<Vec<JoinHandle<()>>>,
}

impl fmt::Debug for AtomicBatchPool {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter
            .debug_struct("AtomicBatchPool")
            .field("thread_count", &self.thread_count())
            .finish_non_exhaustive()
    }
}

impl AtomicBatchPool {
    /// Creates exactly `thread_count` persistent workers.
    ///
    /// # Errors
    ///
    /// Returns `InvalidInput` for zero workers or the underlying thread-spawn
    /// failure. A partial construction is synchronously stopped and joined.
    pub fn new(thread_count: usize) -> io::Result<Self> {
        if thread_count == 0 {
            return Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "atomic batch pool requires at least one worker",
            ));
        }
        let core = Arc::new(Core {
            sleep: Mutex::new(SleepState { terminate: false }),
            run: Mutex::new(()),
            work: Condvar::new(),
            done: Condvar::new(),
            batch: AtomicPtr::new(ptr::null_mut()),
            remaining: AtomicU64::new(0),
            pending: AtomicU32::new(0),
            claim_size: AtomicU32::new(1),
        });
        let mut workers = Vec::with_capacity(thread_count);
        for index in 0..thread_count {
            let worker_core = Arc::clone(&core);
            match thread::Builder::new()
                .name(format!("orc-batch-{index}"))
                .spawn(move || worker_loop(&worker_core))
            {
                Ok(worker) => workers.push(worker),
                Err(error) => {
                    request_termination(&core);
                    for worker in workers {
                        let _ = worker.join();
                    }
                    return Err(error);
                }
            }
        }
        Ok(Self {
            core,
            workers: Mutex::new(workers),
        })
    }

    #[must_use]
    pub fn thread_count(&self) -> usize {
        lock_unpoisoned(&self.workers).len()
    }

    /// Applies one shared function to every distinct task exactly once.
    ///
    /// The function runs serially on the caller below the measured crossover
    /// and may run concurrently on any pool worker above it. Task panics are
    /// contained, the remaining tasks still drain, and the batch returns a
    /// counted error; the pool remains reusable.
    ///
    /// # Errors
    ///
    /// Rejects zero-sized task types, batches outside the u32 ticket domain,
    /// or a batch in which one or more task invocations panic.
    pub fn for_each<T, F>(&self, tasks: &mut [T], function: F) -> Result<(), BatchRunError>
    where
        T: Send,
        F: Fn(&mut T) + Sync,
    {
        if tasks.is_empty() {
            return Ok(());
        }
        if std::mem::size_of::<T>() == 0 {
            return Err(BatchRunError::ZeroSizedTask);
        }
        let count = u32::try_from(tasks.len()).map_err(|_| BatchRunError::TooManyTasks)?;
        debug_assert!(tasks.len() <= MAXIMUM_TASKS);

        let _run = lock_unpoisoned(&self.core.run);
        let worker_count = self.thread_count();
        if worker_count == 1 || tasks.len() <= worker_count.saturating_mul(MINIMUM_TASKS_PER_WORKER)
        {
            let mut panics = 0_u32;
            for task in tasks {
                if catch_unwind(AssertUnwindSafe(|| function(task))).is_err() {
                    panics = panics.saturating_add(1);
                }
            }
            return if panics == 0 {
                Ok(())
            } else {
                Err(BatchRunError::TaskPanicked { count: panics })
            };
        }
        let mut batch = ErasedBatch {
            tasks: tasks.as_mut_ptr().cast(),
            function: ptr::from_ref(&function).cast(),
            call: call_one::<T, F>,
            panics: AtomicU32::new(0),
            _borrowed: PhantomData,
        };
        let mut sleep = lock_unpoisoned(&self.core.sleep);
        debug_assert!(!sleep.terminate);
        self.core
            .batch
            .store(ptr::from_mut(&mut batch), Ordering::Relaxed);
        self.core.pending.store(count, Ordering::Relaxed);
        self.core
            .claim_size
            .store(claim_size(count, worker_count), Ordering::Relaxed);
        self.core
            .remaining
            .store(u64::from(count), Ordering::Release);
        self.core.work.notify_all();

        while self.core.pending.load(Ordering::Acquire) != 0 {
            sleep = wait_unpoisoned(&self.core.done, sleep);
        }
        self.core.batch.store(ptr::null_mut(), Ordering::Relaxed);
        drop(sleep);

        let panics = batch.panics.load(Ordering::Relaxed);
        if panics == 0 {
            Ok(())
        } else {
            Err(BatchRunError::TaskPanicked { count: panics })
        }
    }
}

impl Drop for AtomicBatchPool {
    fn drop(&mut self) {
        let _run = lock_unpoisoned(&self.core.run);
        request_termination(&self.core);
        let workers = std::mem::take(&mut *lock_unpoisoned(&self.workers));
        for worker in workers {
            let _ = worker.join();
        }
    }
}

fn valid_ticket(ticket: u64) -> bool {
    ticket != 0 && u32::try_from(ticket).is_ok()
}

fn claim_size(task_count: u32, worker_count: usize) -> u32 {
    let target_claims = u32::try_from(worker_count)
        .unwrap_or(u32::MAX)
        .saturating_mul(CLAIMS_PER_WORKER_TARGET)
        .max(1);
    task_count
        .div_ceil(target_claims)
        .clamp(1, MAXIMUM_CLAIM_SIZE)
}

fn worker_loop(core: &Core) {
    loop {
        let mut sleep = lock_unpoisoned(&core.sleep);
        while !sleep.terminate && !valid_ticket(core.remaining.load(Ordering::Relaxed)) {
            sleep = wait_unpoisoned(&core.work, sleep);
        }
        if sleep.terminate {
            return;
        }
        drop(sleep);

        let ticket = core.remaining.fetch_sub(1, Ordering::Acquire);
        if !valid_ticket(ticket) {
            continue;
        }
        let batch = core.batch.load(Ordering::Relaxed);
        if batch.is_null() {
            // Publication ordering makes this unreachable. Terminating the
            // worker is safer than dereferencing a violated invariant.
            request_termination(core);
            return;
        }

        let claim_size = u64::from(core.claim_size.load(Ordering::Relaxed));
        let mut local_done = 1_u32;
        let first_index = usize::try_from(ticket - 1).expect("valid ticket fits usize");
        // SAFETY: `for_each` publishes `batch` before the release store to
        // `remaining` and does not return or move its borrowed slice/function
        // until `pending` reaches zero. This worker's acquire claim observes
        // that publication. Ticket uniqueness gives exclusive access to the
        // indexed `T`; the function is shared only through `F: Sync`.
        unsafe {
            ((*batch).call)(
                (*batch).tasks,
                (*batch).function,
                first_index,
                &(*batch).panics,
            );
        }

        if ticket != 1 {
            loop {
                let claim_end = core.remaining.fetch_sub(claim_size, Ordering::Relaxed);
                if !valid_ticket(claim_end) {
                    break;
                }
                let claimed = claim_end.min(claim_size);
                let claim_start = claim_end - claimed;
                for claimed_ticket in (claim_start + 1..=claim_end).rev() {
                    let index =
                        usize::try_from(claimed_ticket - 1).expect("valid ticket fits usize");
                    // SAFETY: `for_each` publishes `batch` before the release store to
                    // `remaining` and does not return or move its borrowed slice/function
                    // until `pending` reaches zero. This worker's acquire claim observes
                    // that publication. Ticket uniqueness gives exclusive access to the
                    // indexed `T`; the function is shared only through `F: Sync`.
                    unsafe {
                        ((*batch).call)((*batch).tasks, (*batch).function, index, &(*batch).panics);
                    }
                }
                local_done += u32::try_from(claimed).expect("claim size fits u32");
                if claim_end <= claim_size {
                    break;
                }
            }
        }

        let before = core.pending.fetch_sub(local_done, Ordering::Release);
        assert!(before >= local_done, "atomic batch completion underflow");
        if before == local_done {
            let _sleep = lock_unpoisoned(&core.sleep);
            core.done.notify_one();
        }
    }
}

unsafe fn call_one<T, F>(tasks: *mut (), function: *const (), index: usize, panics: &AtomicU32)
where
    T: Send,
    F: Fn(&mut T) + Sync,
{
    // SAFETY: established by the caller in `worker_loop`; this function only
    // performs the erased casts and invokes the indexed task.
    let task = unsafe { &mut *tasks.cast::<T>().add(index) };
    // SAFETY: the erased pointer names the live `F` borrowed by `for_each`.
    let function = unsafe { &*function.cast::<F>() };
    if catch_unwind(AssertUnwindSafe(|| function(task))).is_err() {
        let _ = panics.fetch_update(Ordering::Relaxed, Ordering::Relaxed, |value| {
            Some(value.saturating_add(1))
        });
    }
}

fn request_termination(core: &Core) {
    let mut sleep = lock_unpoisoned(&core.sleep);
    sleep.terminate = true;
    core.work.notify_all();
    core.done.notify_all();
}

fn lock_unpoisoned<T>(mutex: &Mutex<T>) -> MutexGuard<'_, T> {
    mutex
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner)
}

fn wait_unpoisoned<'a, T>(condition: &Condvar, guard: MutexGuard<'a, T>) -> MutexGuard<'a, T> {
    condition
        .wait(guard)
        .unwrap_or_else(std::sync::PoisonError::into_inner)
}

#[cfg(test)]
mod tests {
    use super::{AtomicBatchPool, BatchRunError};
    use std::collections::{HashSet, VecDeque};
    use std::sync::Arc;
    use std::sync::atomic::{AtomicUsize, Ordering};
    use std::thread;

    #[derive(Clone, Debug, Hash, PartialEq, Eq)]
    struct ModelState {
        remaining: u8,
        pending: u8,
        owner: [u8; 6],
        completed: u8,
    }

    #[test]
    fn bounded_model_checks_range_claims_and_completion_orders() {
        const TASKS: u8 = 6;
        const WORKERS: u8 = 3;
        const MAX_CLAIM: u8 = 3;
        let initial = ModelState {
            remaining: TASKS,
            pending: TASKS,
            owner: [0; 6],
            completed: 0,
        };
        let mut seen = HashSet::from([initial.clone()]);
        let mut frontier = VecDeque::from([initial]);
        let mut terminal_states = 0_u32;

        while let Some(state) = frontier.pop_front() {
            let claimed = state.owner.iter().filter(|owner| **owner != 0).count();
            assert_eq!(claimed, usize::from(TASKS - state.remaining));
            assert_eq!(
                state.pending,
                TASKS - u8::try_from(state.completed.count_ones()).expect("six-bit count")
            );
            for index in 0..usize::from(TASKS) {
                if state.completed & (1 << index) != 0 {
                    assert_ne!(state.owner[index], 0);
                }
            }

            if state.remaining == 0 && state.pending == 0 {
                assert!(state.owner.iter().all(|owner| *owner != 0));
                terminal_states += 1;
                continue;
            }

            if state.remaining != 0 {
                for worker in 1..=WORKERS {
                    for amount in 1..=MAX_CLAIM {
                        let mut next = state.clone();
                        let taken = amount.min(next.remaining);
                        let start = next.remaining - taken;
                        for ticket in start..next.remaining {
                            let index = usize::from(ticket);
                            assert_eq!(next.owner[index], 0);
                            next.owner[index] = worker;
                        }
                        next.remaining -= taken;
                        if seen.insert(next.clone()) {
                            frontier.push_back(next);
                        }
                    }
                }
            }

            for index in 0..usize::from(TASKS) {
                let bit = 1_u8 << index;
                if state.owner[index] != 0 && state.completed & bit == 0 {
                    let mut next = state.clone();
                    next.completed |= bit;
                    next.pending -= 1;
                    if seen.insert(next.clone()) {
                        frontier.push_back(next);
                    }
                }
            }
        }

        assert!(terminal_states > 0);
    }

    #[test]
    fn every_ticket_is_claimed_once_and_writes_are_visible() {
        let pool = AtomicBatchPool::new(4).expect("pool");
        let mut values = vec![0_u64; 10_000];
        pool.for_each(&mut values, |value| *value += 1)
            .expect("batch");
        assert!(values.iter().all(|value| *value == 1));
    }

    #[test]
    fn concurrent_callers_are_serialized_without_losing_work() {
        let pool = Arc::new(AtomicBatchPool::new(4).expect("pool"));
        let entered = Arc::new(AtomicUsize::new(0));
        let mut callers = Vec::new();
        for _ in 0..4 {
            let pool = Arc::clone(&pool);
            let entered = Arc::clone(&entered);
            callers.push(thread::spawn(move || {
                let mut values = vec![0_u32; 2_048];
                pool.for_each(&mut values, |value| {
                    *value = 1;
                    entered.fetch_add(1, Ordering::Relaxed);
                })
                .expect("serialized batch");
                values
            }));
        }
        for caller in callers {
            assert!(
                caller
                    .join()
                    .expect("caller")
                    .iter()
                    .all(|value| *value == 1)
            );
        }
        assert_eq!(entered.load(Ordering::Relaxed), 8_192);
    }

    #[test]
    fn task_panics_are_counted_and_pool_remains_reusable() {
        let pool = AtomicBatchPool::new(3).expect("pool");
        let mut values = (0..1_024_u32).collect::<Vec<_>>();
        let result = pool.for_each(&mut values, |value| {
            let original = *value;
            *value += 1;
            assert_ne!(original % 257, 0, "contained fixture panic");
        });
        assert_eq!(result, Err(BatchRunError::TaskPanicked { count: 4 }));
        pool.for_each(&mut values, |value| *value += 1)
            .expect("pool survives task panic");
        assert!(
            values
                .iter()
                .enumerate()
                .all(|(index, value)| *value == u32::try_from(index).expect("index") + 2)
        );
    }

    #[test]
    fn caller_lane_panics_are_counted_and_pool_remains_reusable() {
        let pool = AtomicBatchPool::new(4).expect("pool");
        let mut values = [0_u32; 8];
        let result = pool.for_each(&mut values, |value| {
            *value = 1;
            panic!("contained caller-lane fixture panic");
        });
        assert_eq!(result, Err(BatchRunError::TaskPanicked { count: 8 }));
        pool.for_each(&mut values, |value| *value += 1)
            .expect("pool survives caller-lane task panic");
        assert_eq!(values, [2; 8]);
    }

    #[test]
    fn empty_is_a_noop_and_zero_sized_tasks_are_rejected() {
        let pool = AtomicBatchPool::new(1).expect("pool");
        let mut empty: [u64; 0] = [];
        pool.for_each(&mut empty, |_| unreachable!())
            .expect("empty");
        let mut zero_sized = [(); 1];
        assert_eq!(
            pool.for_each(&mut zero_sized, |()| {}),
            Err(BatchRunError::ZeroSizedTask)
        );
    }

    #[test]
    fn repeated_parallel_batches_cross_ticket_underflow_and_republication() {
        let pool = AtomicBatchPool::new(8).expect("pool");
        for round in 1..=2_000_u32 {
            let mut values = vec![round; 513];
            pool.for_each(&mut values, |value| *value ^= round)
                .expect("parallel batch");
            assert!(values.iter().all(|value| *value == 0));
        }
    }
}
