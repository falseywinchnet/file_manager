//! Selected-preview lifecycle laboratory. All identifiers are inert fixture
//! values, not capabilities or filesystem identity implementations. No provider
//! is opened, launched or advertised. See the adjacent README and draft profile.

pub const CLIENT_LIMIT: usize = 8;
pub const DECODE_MILLISECONDS: u64 = 3_000;
pub const ACK_MILLISECONDS: u64 = 1_000;
pub const RASTER_BYTE_LIMIT: u64 = 4 * 1024 * 1024;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Ticket {
    pub session: u64,
    pub nonce: u64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Source {
    pub object: u64,
    pub revision: u64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Request {
    pub window: u64,
    pub ticket: Ticket,
    pub source: Source,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Color {
    SrgbOpaqueBgra8,
    Unsupported,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Raster {
    pub width: u32,
    pub height: u32,
    pub stride: u32,
    pub payload_bytes: u64,
    pub color: Color,
    pub orientation_applied: bool,
}

impl Raster {
    /// Descriptor validation only. Byte contents and payload ownership are not
    /// modeled; an adapter must separately validate the actual received extent.
    pub fn valid(&self) -> bool {
        if self.width == 0 || self.height == 0 || self.width > 1024 || self.height > 1024 {
            return false;
        }
        let expected_stride: u32 = self.width * 4;
        let expected_bytes: u64 = u64::from(expected_stride) * u64::from(self.height);
        let valid: bool = self.stride == expected_stride
            && self.payload_bytes == expected_bytes
            && expected_bytes <= RASTER_BYTE_LIMIT
            && self.color == Color::SrgbOpaqueBgra8
            && self.orientation_applied;
        valid
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Phase {
    Idle,
    Running {
        request: Request,
        deadline: u64,
    },
    Stopping {
        request: Request,
        reason: Reason,
    },
    Offered {
        request: Request,
        raster: Raster,
        expires: u64,
    },
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Reason {
    Cancelled,
    TimedOut,
    AckExpired,
    StaleSource,
    InvalidRaster,
    DecodeFailed,
    ClockExhausted,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Error {
    InvalidIdentity,
    DuplicateClient,
    ClientLimit,
    UnknownClient,
    WrongTicket,
    WrongPhase,
    ClockReversed,
    CounterExhausted,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Terminal {
    pub ticket: Ticket,
    pub reason: Reason,
}

/// Instructions for a future adapter. Retirement means releasing its owned
/// pixels/grant and notifying the client; stop means terminate, then reap.
/// Dropping an effect without executing it is not conformance.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Effects {
    pub stop: Option<Ticket>,
    pub retire_offer: Option<Ticket>,
    pub retire_display: Option<Ticket>,
    pub terminal: Option<Terminal>,
}

impl Effects {
    fn empty() -> Self {
        Self {
            stop: None,
            retire_offer: None,
            retire_display: None,
            terminal: None,
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Submission {
    pub request: Request,
    pub effects: Effects,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Completion {
    Pixels { source: Source, raster: Raster },
    Failed,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Reaped {
    Offered,
    Discarded(Reason),
}

#[derive(Clone, Copy)]
struct Display {
    request: Request,
    bytes: u64,
}

#[derive(Clone, Copy)]
struct Client {
    window: u64,
    pending: Option<Request>,
    current: Option<Ticket>,
    display: Option<Display>,
}

pub struct Broker {
    session: u64,
    nonce: u64,
    now: u64,
    next_client: usize,
    clients: [Option<Client>; CLIENT_LIMIT],
    phase: Phase,
}

impl Broker {
    pub fn new(session: u64) -> Result<Self, Error> {
        if session == 0 {
            return Err(Error::InvalidIdentity);
        }
        let broker: Self = Self {
            session,
            nonce: 0,
            now: 0,
            next_client: 0,
            clients: [None; CLIENT_LIMIT],
            phase: Phase::Idle,
        };
        Ok(broker)
    }

    pub fn phase(&self) -> Phase {
        self.phase
    }

    fn client_index(&self, window: u64) -> Result<usize, Error> {
        for index in 0..CLIENT_LIMIT {
            if let Some(client) = self.clients[index] {
                if client.window == window {
                    return Ok(index);
                }
            }
        }
        Err(Error::UnknownClient)
    }

    pub fn connect(&mut self, window: u64) -> Result<(), Error> {
        if window == 0 {
            return Err(Error::InvalidIdentity);
        }
        if self.client_index(window).is_ok() {
            return Err(Error::DuplicateClient);
        }
        for index in 0..CLIENT_LIMIT {
            if self.clients[index].is_none() {
                self.clients[index] = Some(Client {
                    window,
                    pending: None,
                    current: None,
                    display: None,
                });
                return Ok(());
            }
        }
        Err(Error::ClientLimit)
    }

    // Caller already established a live client index. No allocations or retained
    // borrows: copy its small inert state, calculate retirements, then publish.
    fn cancel_at(&mut self, index: usize) -> Effects {
        let mut effects: Effects = Effects::empty();
        if let Some(mut client) = self.clients[index] {
            if let Some(pending) = client.pending {
                effects.terminal = Some(Terminal {
                    ticket: pending.ticket,
                    reason: Reason::Cancelled,
                });
            }
            if let Some(display) = client.display {
                effects.retire_display = Some(display.request.ticket);
            }
            client.pending = None;
            client.current = None;
            client.display = None;
            match self.phase {
                Phase::Running { request, .. } if request.window == client.window => {
                    self.phase = Phase::Stopping {
                        request,
                        reason: Reason::Cancelled,
                    };
                    effects.stop = Some(request.ticket);
                }
                Phase::Offered { request, .. } if request.window == client.window => {
                    self.phase = Phase::Idle;
                    effects.retire_offer = Some(request.ticket);
                    effects.terminal = Some(Terminal {
                        ticket: request.ticket,
                        reason: Reason::Cancelled,
                    });
                }
                _ => {}
            }
            self.clients[index] = Some(client);
        }
        effects
    }

    pub fn cancel(&mut self, window: u64) -> Result<Effects, Error> {
        let index: usize = self.client_index(window)?;
        let effects: Effects = self.cancel_at(index);
        Ok(effects)
    }

    pub fn disconnect(&mut self, window: u64) -> Result<Effects, Error> {
        let index: usize = self.client_index(window)?;
        let effects: Effects = self.cancel_at(index);
        self.clients[index] = None;
        Ok(effects)
    }

    pub fn submit(&mut self, window: u64, source: Source) -> Result<Submission, Error> {
        let index: usize = self.client_index(window)?;
        if source.object == 0 || source.revision == 0 {
            return Err(Error::InvalidIdentity);
        }
        let nonce: u64 = self.nonce.checked_add(1).ok_or(Error::CounterExhausted)?;
        let request: Request = Request {
            window,
            ticket: Ticket {
                session: self.session,
                nonce,
            },
            source,
        };
        let effects: Effects = self.cancel_at(index);
        self.nonce = nonce;
        self.clients[index] = Some(Client {
            window,
            pending: Some(request),
            current: Some(request.ticket),
            display: None,
        });
        let submission: Submission = Submission { request, effects };
        Ok(submission)
    }

    /// Advance the monotonic fixture clock before processing each batch of
    /// external events. The future host must execute returned effects first.
    /// The laboratory does not schedule wakeups or impose an OS deadline.
    pub fn advance(&mut self, now: u64) -> Result<Effects, Error> {
        if now < self.now {
            return Err(Error::ClockReversed);
        }
        let mut effects: Effects = Effects::empty();
        match self.phase {
            Phase::Running { request, deadline } if now >= deadline => {
                self.phase = Phase::Stopping {
                    request,
                    reason: Reason::TimedOut,
                };
                effects.stop = Some(request.ticket);
            }
            Phase::Offered {
                request, expires, ..
            } if now >= expires => {
                self.phase = Phase::Idle;
                effects.retire_offer = Some(request.ticket);
                effects.terminal = Some(Terminal {
                    ticket: request.ticket,
                    reason: Reason::AckExpired,
                });
            }
            _ => {}
        }
        self.now = now;
        Ok(effects)
    }

    pub fn dispatch(&mut self) -> Result<Option<Request>, Error> {
        if self.phase != Phase::Idle {
            return Ok(None);
        }
        for offset in 0..CLIENT_LIMIT {
            let index: usize = (self.next_client + offset) % CLIENT_LIMIT;
            if let Some(mut client) = self.clients[index] {
                if let Some(request) = client.pending {
                    let deadline: u64 = self
                        .now
                        .checked_add(DECODE_MILLISECONDS)
                        .ok_or(Error::CounterExhausted)?;
                    client.pending = None;
                    self.clients[index] = Some(client);
                    self.phase = Phase::Running { request, deadline };
                    self.next_client = (index + 1) % CLIENT_LIMIT;
                    return Ok(Some(request));
                }
            }
        }
        Ok(None)
    }

    /// This event means the matching process has exited AND been reaped. A
    /// decoder result, EOF or successful termination request alone is not enough.
    /// Source is a trusted host's post-read observation in this abstract model.
    pub fn reaped(&mut self, ticket: Ticket, completion: Completion) -> Result<Reaped, Error> {
        let request: Request = match self.phase {
            Phase::Running { request, .. } | Phase::Stopping { request, .. } => request,
            _ => return Err(Error::WrongPhase),
        };
        if request.ticket != ticket {
            return Err(Error::WrongTicket);
        }
        if let Phase::Stopping { reason, .. } = self.phase {
            self.phase = Phase::Idle;
            return Ok(Reaped::Discarded(reason));
        }
        let outcome: Reaped = match completion {
            Completion::Failed => Reaped::Discarded(Reason::DecodeFailed),
            Completion::Pixels { source, raster } => {
                if source != request.source {
                    Reaped::Discarded(Reason::StaleSource)
                } else if !raster.valid() {
                    Reaped::Discarded(Reason::InvalidRaster)
                } else {
                    if let Some(expires) = self.now.checked_add(ACK_MILLISECONDS) {
                        self.phase = Phase::Offered {
                            request,
                            raster,
                            expires,
                        };
                        return Ok(Reaped::Offered);
                    }
                    Reaped::Discarded(Reason::ClockExhausted)
                }
            }
        };
        self.phase = Phase::Idle;
        Ok(outcome)
    }

    pub fn acknowledge(&mut self, window: u64, ticket: Ticket) -> Result<(), Error> {
        let index: usize = self.client_index(window)?;
        let (request, raster): (Request, Raster) = match self.phase {
            Phase::Offered {
                request, raster, ..
            } => (request, raster),
            _ => return Err(Error::WrongPhase),
        };
        if request.window != window || request.ticket != ticket {
            return Err(Error::WrongTicket);
        }
        if let Some(mut client) = self.clients[index] {
            if client.current != Some(ticket) {
                return Err(Error::WrongTicket);
            }
            client.display = Some(Display {
                request,
                bytes: raster.payload_bytes,
            });
            self.clients[index] = Some(client);
        }
        self.phase = Phase::Idle;
        Ok(())
    }

    pub fn display_bytes(&self) -> u64 {
        let mut total: u64 = 0;
        for index in 0..CLIENT_LIMIT {
            if let Some(client) = self.clients[index] {
                if let Some(display) = client.display {
                    total += display.bytes;
                }
            }
        }
        total
    }

    pub fn pending_count(&self) -> usize {
        let mut count: usize = 0;
        for index in 0..CLIENT_LIMIT {
            if let Some(client) = self.clients[index] {
                if client.pending.is_some() {
                    count += 1;
                }
            }
        }
        count
    }
}
