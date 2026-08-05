use serde::{Deserialize, Serialize};
use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum LifecycleState {
    Created,
    Starting,
    Ready,
    Draining,
    Stopped,
    Faulted,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LifecycleError {
    from: LifecycleState,
    to: LifecycleState,
}

impl fmt::Display for LifecycleError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            formatter,
            "invalid lifecycle transition from {:?} to {:?}",
            self.from, self.to
        )
    }
}

impl std::error::Error for LifecycleError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
pub struct LifecycleSnapshot {
    pub state: LifecycleState,
    pub generation: u64,
}

#[derive(Debug, Clone)]
pub struct Lifecycle {
    state: LifecycleState,
    generation: u64,
}

impl Default for Lifecycle {
    fn default() -> Self {
        Self {
            state: LifecycleState::Created,
            generation: 0,
        }
    }
}

impl Lifecycle {
    /// Creates a lifecycle that has completed its first deterministic start.
    #[must_use]
    pub const fn started() -> Self {
        Self {
            state: LifecycleState::Ready,
            generation: 1,
        }
    }

    #[must_use]
    pub fn snapshot(&self) -> LifecycleSnapshot {
        LifecycleSnapshot {
            state: self.state,
            generation: self.generation,
        }
    }

    /// Starts or restarts the service.
    ///
    /// # Errors
    ///
    /// Returns an error unless the service is newly created or stopped.
    pub fn start(&mut self) -> Result<(), LifecycleError> {
        self.transition(LifecycleState::Starting)?;
        self.generation = self.generation.saturating_add(1);
        self.transition(LifecycleState::Ready)
    }

    /// Moves a ready service into its draining state.
    ///
    /// # Errors
    ///
    /// Returns an error unless the service is ready.
    pub fn begin_shutdown(&mut self) -> Result<(), LifecycleError> {
        self.transition(LifecycleState::Draining)
    }

    /// Completes a shutdown after draining.
    ///
    /// # Errors
    ///
    /// Returns an error unless the service is draining.
    pub fn finish_shutdown(&mut self) -> Result<(), LifecycleError> {
        self.transition(LifecycleState::Stopped)
    }

    /// Marks an active lifecycle as faulted.
    ///
    /// # Errors
    ///
    /// Returns an error when the lifecycle is already stopped or faulted.
    pub fn fault(&mut self) -> Result<(), LifecycleError> {
        self.transition(LifecycleState::Faulted)
    }

    fn transition(&mut self, next: LifecycleState) -> Result<(), LifecycleError> {
        let allowed = matches!(
            (self.state, next),
            (
                LifecycleState::Created | LifecycleState::Stopped,
                LifecycleState::Starting,
            ) | (
                LifecycleState::Starting,
                LifecycleState::Ready | LifecycleState::Faulted,
            ) | (
                LifecycleState::Ready,
                LifecycleState::Draining | LifecycleState::Faulted,
            ) | (
                LifecycleState::Draining,
                LifecycleState::Stopped | LifecycleState::Faulted,
            ) | (LifecycleState::Created, LifecycleState::Faulted)
        );
        if !allowed {
            return Err(LifecycleError {
                from: self.state,
                to: next,
            });
        }
        self.state = next;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::{Lifecycle, LifecycleState};

    #[test]
    fn lifecycle_is_deterministic_and_restartable() {
        let mut lifecycle = Lifecycle::default();
        lifecycle.start().expect("first start");
        assert_eq!(lifecycle.snapshot().state, LifecycleState::Ready);
        assert_eq!(lifecycle.snapshot().generation, 1);
        lifecycle.begin_shutdown().expect("begin shutdown");
        lifecycle.finish_shutdown().expect("finish shutdown");
        lifecycle.start().expect("restart");
        assert_eq!(lifecycle.snapshot().generation, 2);
    }

    #[test]
    fn invalid_transition_is_rejected() {
        let mut lifecycle = Lifecycle::default();
        assert!(lifecycle.begin_shutdown().is_err());
        assert_eq!(lifecycle.snapshot().state, LifecycleState::Created);
    }
}
