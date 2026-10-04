pub const HEADER_BYTES: usize = 64;
pub const MAX_RASTER_BYTES: usize = 4 * 1024 * 1024;
const MAGIC: [u8; 8] = *b"FMPREV01";

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Ticket {
    pub session: u64,
    pub nonce: u64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Status {
    Ready,
    Unsupported,
    InvalidInput,
    ResourceLimit,
    DecodeFailed,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Error {
    InvalidTicket,
    Header,
    Version,
    WrongTicket,
    Status,
    Geometry,
    Profile,
    Reserved,
    Allocation,
    TrailingBytes,
    Truncated,
    NonOpaque,
    Poisoned,
}

#[derive(Debug)]
pub struct Raster {
    pub ticket: Ticket,
    pub width: u32,
    pub height: u32,
    // Owned tight, top-to-bottom, opaque sRGB BGRA8 bytes, orientation applied.
    pub pixels: Vec<u8>,
}

#[derive(Debug)]
pub enum Frame {
    Raster(Raster),
    Terminal { ticket: Ticket, status: Status },
}

// One frame owner. Input chunks are borrowed only during push. No native layout
// is serialized. Header validation precedes the single bounded raster allocation.
pub struct Receiver {
    ticket: Ticket,
    header: [u8; HEADER_BYTES],
    header_used: usize,
    status: Status,
    width: u32,
    height: u32,
    pixels: Vec<u8>,
    received: usize,
    poisoned: bool,
}

fn read_u16(bytes: &[u8; HEADER_BYTES], offset: usize) -> u16 {
    let value: u16 = u16::from_le_bytes([bytes[offset], bytes[offset + 1]]);
    value
}

fn read_u32(bytes: &[u8; HEADER_BYTES], offset: usize) -> u32 {
    let value: u32 = u32::from_le_bytes([
        bytes[offset],
        bytes[offset + 1],
        bytes[offset + 2],
        bytes[offset + 3],
    ]);
    value
}

fn read_u64(bytes: &[u8; HEADER_BYTES], offset: usize) -> u64 {
    let value: u64 = u64::from_le_bytes([
        bytes[offset],
        bytes[offset + 1],
        bytes[offset + 2],
        bytes[offset + 3],
        bytes[offset + 4],
        bytes[offset + 5],
        bytes[offset + 6],
        bytes[offset + 7],
    ]);
    value
}

impl Receiver {
    pub fn new(ticket: Ticket) -> Result<Self, Error> {
        if ticket.session == 0 || ticket.nonce == 0 {
            return Err(Error::InvalidTicket);
        }
        let receiver: Self = Self {
            ticket,
            header: [0; HEADER_BYTES],
            header_used: 0,
            status: Status::DecodeFailed,
            width: 0,
            height: 0,
            pixels: Vec::new(),
            received: 0,
            poisoned: false,
        };
        Ok(receiver)
    }

    // Consumes the entire supplied chunk or permanently poisons this receiver.
    // Failure releases raster storage. Empty chunks do not represent EOF.
    pub fn push(&mut self, input: &[u8]) -> Result<(), Error> {
        if self.poisoned {
            return Err(Error::Poisoned);
        }
        let result: Result<(), Error> = self.push_inner(input);
        if result.is_err() {
            self.poisoned = true;
            self.pixels = Vec::new();
            self.received = 0;
        }
        result
    }

    fn push_inner(&mut self, input: &[u8]) -> Result<(), Error> {
        let mut consumed: usize = 0;
        if self.header_used < HEADER_BYTES {
            let available: usize = HEADER_BYTES - self.header_used;
            let copied: usize = available.min(input.len());
            let end: usize = self.header_used + copied;
            self.header[self.header_used..end].copy_from_slice(&input[..copied]);
            self.header_used = end;
            consumed = copied;
            if self.header_used < HEADER_BYTES {
                return Ok(());
            }
            self.admit_header()?;
        }
        let body: &[u8] = &input[consumed..];
        let remaining: usize = self.pixels.len() - self.received;
        if body.len() > remaining {
            return Err(Error::TrailingBytes);
        }
        let end: usize = self.received + body.len();
        self.pixels[self.received..end].copy_from_slice(body);
        self.received = end;
        Ok(())
    }

    fn admit_header(&mut self) -> Result<(), Error> {
        if self.header[..8] != MAGIC || read_u16(&self.header, 10) != 64 {
            return Err(Error::Header);
        }
        if read_u16(&self.header, 8) != 1 {
            return Err(Error::Version);
        }
        let session: u64 = read_u64(&self.header, 16);
        let nonce: u64 = read_u64(&self.header, 24);
        if session != self.ticket.session || nonce != self.ticket.nonce {
            return Err(Error::WrongTicket);
        }
        let mut reserved_index: usize = 52;
        while reserved_index < HEADER_BYTES {
            if self.header[reserved_index] != 0 {
                return Err(Error::Reserved);
            }
            reserved_index += 1;
        }
        let status: Status = match read_u32(&self.header, 12) {
            0 => Status::Ready,
            1 => Status::Unsupported,
            2 => Status::InvalidInput,
            3 => Status::ResourceLimit,
            4 => Status::DecodeFailed,
            _ => return Err(Error::Status),
        };
        let width: u32 = read_u32(&self.header, 32);
        let height: u32 = read_u32(&self.header, 36);
        let stride: u32 = read_u32(&self.header, 40);
        let payload: u32 = read_u32(&self.header, 44);
        let profile: u32 = read_u32(&self.header, 48);
        if status != Status::Ready {
            if width != 0 || height != 0 || stride != 0 || payload != 0 || profile != 0 {
                return Err(Error::Geometry);
            }
            self.status = status;
            return Ok(());
        }
        if profile != 1 {
            return Err(Error::Profile);
        }
        if width == 0 || height == 0 || width > 1024 || height > 1024 {
            return Err(Error::Geometry);
        }
        // Admitted axes bound both products before multiplication.
        let expected_stride: u32 = width * 4;
        let expected_payload: u32 = expected_stride * height;
        if stride != expected_stride || payload != expected_payload {
            return Err(Error::Geometry);
        }
        let length: usize = match usize::try_from(payload) {
            Ok(value) => value,
            Err(_) => return Err(Error::Geometry),
        };
        if length > MAX_RASTER_BYTES {
            return Err(Error::Geometry);
        }
        let reserved: Result<(), std::collections::TryReserveError> =
            self.pixels.try_reserve_exact(length);
        if reserved.is_err() {
            return Err(Error::Allocation);
        }
        self.pixels.resize(length, 0);
        self.width = width;
        self.height = height;
        self.status = status;
        Ok(())
    }

    // Call only at actual stream EOF. No raster escapes until exact extent and
    // pixel alpha are validated. A valid frame is not permission to publish:
    // host reaping, ticket freshness, source validation and budget gates remain.
    pub fn finish(self) -> Result<Frame, Error> {
        if self.poisoned {
            return Err(Error::Poisoned);
        }
        if self.header_used != HEADER_BYTES || self.received != self.pixels.len() {
            return Err(Error::Truncated);
        }
        if self.status != Status::Ready {
            let terminal: Frame = Frame::Terminal {
                ticket: self.ticket,
                status: self.status,
            };
            return Ok(terminal);
        }
        let mut alpha_index: usize = 3;
        while alpha_index < self.pixels.len() {
            if self.pixels[alpha_index] != 255 {
                return Err(Error::NonOpaque);
            }
            alpha_index += 4;
        }
        let raster: Raster = Raster {
            ticket: self.ticket,
            width: self.width,
            height: self.height,
            pixels: self.pixels,
        };
        Ok(Frame::Raster(raster))
    }
}
