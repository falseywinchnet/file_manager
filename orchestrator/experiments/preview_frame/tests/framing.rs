use file_manager_preview_frame_lab::{Error, Frame, Receiver, Status, Ticket, HEADER_BYTES};

const TICKET: Ticket = Ticket {
    session: 7,
    nonce: 11,
};

fn write_u32(bytes: &mut [u8], offset: usize, value: u32) {
    let encoded: [u8; 4] = value.to_le_bytes();
    bytes[offset..offset + 4].copy_from_slice(&encoded);
}

fn fixture() -> Vec<u8> {
    let mut bytes: Vec<u8> = vec![0; HEADER_BYTES + 24];
    bytes[..8].copy_from_slice(b"FMPREV01");
    bytes[8] = 1;
    bytes[10] = 64;
    bytes[16] = 7;
    bytes[24] = 11;
    write_u32(&mut bytes, 32, 2);
    write_u32(&mut bytes, 36, 3);
    write_u32(&mut bytes, 40, 8);
    write_u32(&mut bytes, 44, 24);
    write_u32(&mut bytes, 48, 1);
    // Independent six-pixel golden, row-major opaque BGRA; no codec involved.
    bytes[64..].copy_from_slice(&[
        0, 0, 255, 255, 0, 255, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 20, 40,
        60, 255,
    ]);
    bytes
}

fn verify(frame: Frame) {
    match frame {
        Frame::Raster(raster) => {
            assert_eq!(raster.ticket, TICKET);
            assert_eq!(raster.width, 2);
            assert_eq!(raster.height, 3);
            assert_eq!(raster.pixels.len(), 24);
            assert_eq!(&raster.pixels[20..], &[20, 40, 60, 255]);
        }
        Frame::Terminal { .. } => panic!("expected raster"),
    }
}

#[test]
fn every_split_and_single_byte_delivery_preserve_pixels() {
    let bytes: Vec<u8> = fixture();
    for split in 0..=bytes.len() {
        let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
        receiver.push(&bytes[..split]).unwrap();
        receiver.push(&[]).unwrap();
        receiver.push(&bytes[split..]).unwrap();
        let frame: Frame = receiver.finish().unwrap();
        verify(frame);
    }
    let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
    for index in 0..bytes.len() {
        receiver.push(&bytes[index..index + 1]).unwrap();
    }
    let frame: Frame = receiver.finish().unwrap();
    verify(frame);
}

#[test]
fn every_short_extent_is_truncated() {
    let bytes: Vec<u8> = fixture();
    for end in 0..bytes.len() {
        let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
        receiver.push(&bytes[..end]).unwrap();
        let error: Error = receiver.finish().unwrap_err();
        assert_eq!(error, Error::Truncated);
    }
}

#[test]
fn invalid_header_fields_and_extreme_products_are_refused() {
    let fields: [(usize, u32, Error); 12] = [
        (12, 5, Error::Status),
        (16, 8, Error::WrongTicket),
        (24, 12, Error::WrongTicket),
        (32, 0, Error::Geometry),
        (32, 1025, Error::Geometry),
        (36, u32::MAX, Error::Geometry),
        (40, u32::MAX, Error::Geometry),
        (44, u32::MAX, Error::Geometry),
        (40, 9, Error::Geometry),
        (44, 23, Error::Geometry),
        (48, 2, Error::Profile),
        (52, 1, Error::Reserved),
    ];
    for field in &fields {
        let (offset, value, expected): (usize, u32, Error) = *field;
        let mut bytes: Vec<u8> = fixture();
        write_u32(&mut bytes, offset, value);
        let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
        let error: Error = receiver.push(&bytes).unwrap_err();
        assert_eq!(error, expected);
        assert_eq!(receiver.push(&[]), Err(Error::Poisoned));
        assert_eq!(receiver.finish().unwrap_err(), Error::Poisoned);
    }
    let offsets: [(usize, Error); 3] =
        [(0, Error::Header), (8, Error::Version), (10, Error::Header)];
    for field in &offsets {
        let (offset, expected): (usize, Error) = *field;
        let mut bytes: Vec<u8> = fixture();
        bytes[offset] = 9;
        let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
        assert_eq!(receiver.push(&bytes), Err(expected));
    }
}

#[test]
fn trailing_frames_and_nonopaque_bytes_never_publish() {
    let mut bytes: Vec<u8> = fixture();
    let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
    receiver.push(&bytes).unwrap();
    assert_eq!(receiver.push(&bytes), Err(Error::TrailingBytes));
    assert_eq!(receiver.finish().unwrap_err(), Error::Poisoned);
    bytes.push(0);
    let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
    assert_eq!(receiver.push(&bytes), Err(Error::TrailingBytes));
    bytes.pop();
    bytes[67] = 254;
    let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
    receiver.push(&bytes).unwrap();
    assert_eq!(receiver.finish().unwrap_err(), Error::NonOpaque);
}

#[test]
fn terminal_statuses_require_empty_geometry_and_payload() {
    let statuses: [Status; 4] = [
        Status::Unsupported,
        Status::InvalidInput,
        Status::ResourceLimit,
        Status::DecodeFailed,
    ];
    let mut index: usize = 0;
    while index < statuses.len() {
        let mut bytes: Vec<u8> = fixture();
        bytes.truncate(HEADER_BYTES);
        bytes[32..].fill(0);
        let status_code: u32 = u32::try_from(index + 1).unwrap();
        write_u32(&mut bytes, 12, status_code);
        let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
        receiver.push(&bytes).unwrap();
        match receiver.finish().unwrap() {
            Frame::Terminal { ticket, status } => {
                assert_eq!(ticket, TICKET);
                assert_eq!(status, statuses[index]);
            }
            Frame::Raster(_) => panic!("expected terminal"),
        }
        bytes[44] = 4;
        let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
        assert_eq!(receiver.push(&bytes), Err(Error::Geometry));
        index += 1;
    }
}

#[test]
fn maximum_raster_is_admitted_and_zero_tickets_are_refused() {
    let mut bytes: Vec<u8> = fixture();
    write_u32(&mut bytes, 32, 1024);
    write_u32(&mut bytes, 36, 1024);
    write_u32(&mut bytes, 40, 4096);
    write_u32(&mut bytes, 44, 4 * 1024 * 1024);
    bytes.resize(HEADER_BYTES + 4 * 1024 * 1024, 255);
    let mut receiver: Receiver = Receiver::new(TICKET).unwrap();
    receiver.push(&bytes).unwrap();
    let frame: Frame = receiver.finish().unwrap();
    match frame {
        Frame::Raster(raster) => assert_eq!(raster.pixels.len(), 4 * 1024 * 1024),
        Frame::Terminal { .. } => panic!("expected raster"),
    }
    let invalid: Ticket = Ticket {
        session: 0,
        nonce: 1,
    };
    assert!(matches!(Receiver::new(invalid), Err(Error::InvalidTicket)));
    let invalid: Ticket = Ticket {
        session: 1,
        nonce: 0,
    };
    assert!(matches!(Receiver::new(invalid), Err(Error::InvalidTicket)));
}
