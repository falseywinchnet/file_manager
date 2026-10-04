use file_manager_preview_frame_lab::{Frame, Raster, Receiver, Ticket};
use std::io::Read;

fn fail(message: &str) -> String {
    let result: String = String::from(message);
    result
}

fn read_raster() -> Result<Raster, String> {
    let ticket: Ticket = Ticket {
        session: 7,
        nonce: 11,
    };
    let mut receiver: Receiver = match Receiver::new(ticket) {
        Ok(value) => value,
        Err(_) => return Err(fail("invalid fixed ticket")),
    };
    let stdin: std::io::Stdin = std::io::stdin();
    let mut reader: std::io::StdinLock<'_> = stdin.lock();
    let mut chunk: [u8; 4096] = [0; 4096];
    loop {
        let count: usize = match reader.read(&mut chunk) {
            Ok(0) => break,
            Ok(value) => value,
            Err(error) if error.kind() == std::io::ErrorKind::Interrupted => continue,
            Err(_) => return Err(fail("JPEG frame stream read failed")),
        };
        if let Err(error) = receiver.push(&chunk[..count]) {
            let message: String = format!("JPEG frame rejected: {error:?}");
            return Err(message);
        }
    }
    let frame: Frame = match receiver.finish() {
        Ok(value) => value,
        Err(error) => {
            let message: String = format!("JPEG frame finish rejected: {error:?}");
            return Err(message);
        }
    };
    match frame {
        Frame::Raster(raster) => Ok(raster),
        Frame::Terminal { .. } => Err(fail("expected a generated JPEG raster")),
    }
}

// Independent scalar transfer curve. Fixture inputs are 24..240, so the rounded
// result is finite and representable in i32. Tolerance allows JPEG loss and LCMS.
fn encode_srgb(sample: i32) -> i32 {
    let linear: f64 = f64::from(sample) / 255.0;
    let encoded: f64 = if linear <= 0.0031308 {
        linear * 12.92
    } else {
        1.055 * linear.powf(1.0 / 2.4) - 0.055
    };
    let scaled: f64 = encoded * 255.0;
    let rounded: f64 = scaled.round();
    let result: i32 = rounded as i32;
    result
}

fn verify(raster: &Raster, orientation: usize, linear: bool) -> Result<(), String> {
    let width: u32 = if orientation < 5 { 1024 } else { 768 };
    let height: u32 = if orientation < 5 { 768 } else { 1024 };
    if raster.width != width || raster.height != height {
        return Err(fail("scaled/oriented JPEG extent differs"));
    }
    // Output TL/TR/BL/BR labels derived from the eight EXIF transformations.
    const QUADRANTS: [[usize; 4]; 8] = [
        [0, 1, 2, 3],
        [1, 0, 3, 2],
        [3, 2, 1, 0],
        [2, 3, 0, 1],
        [0, 2, 1, 3],
        [2, 0, 3, 1],
        [3, 1, 2, 0],
        [1, 3, 0, 2],
    ];
    let mut colors: [[i32; 3]; 4] = [[24, 24, 240], [24, 232, 24], [224, 24, 24], [24, 216, 216]];
    if linear {
        for color in &mut colors {
            for sample in color {
                let encoded: i32 = encode_srgb(*sample);
                *sample = encoded;
            }
        }
    }
    let mut corner: usize = 0;
    while corner < 4 {
        let x: u32 = if corner == 0 || corner == 2 {
            width / 4
        } else {
            3 * width / 4
        };
        let y: u32 = if corner < 2 {
            height / 4
        } else {
            3 * height / 4
        };
        // Fixed accepted dimensions bound this product to less than 4 MiB.
        let byte_offset: u32 = (y * width + x) * 4;
        let offset: usize = match usize::try_from(byte_offset) {
            Ok(value) => value,
            Err(_) => return Err(fail("fixture offset conversion failed")),
        };
        let quadrant: usize = QUADRANTS[orientation - 1][corner];
        let mut channel: usize = 0;
        while channel < 3 {
            let actual: i32 = i32::from(raster.pixels[offset + channel]);
            let difference: i32 = actual - colors[quadrant][channel];
            if !(-4..=4).contains(&difference) {
                let message: String = format!("JPEG color/orientation differs: corner {corner}, channel {channel}, delta {difference}");
                return Err(message);
            }
            channel += 1;
        }
        corner += 1;
    }
    Ok(())
}

fn execute() -> Result<(), String> {
    let mut arguments: std::env::Args = std::env::args();
    let _program: Option<String> = arguments.next();
    let orientation_text: String = match arguments.next() {
        Some(value) => value,
        None => return Err(fail("orientation required")),
    };
    let orientation: usize = match orientation_text.parse::<usize>() {
        Ok(value) if (1..=8).contains(&value) => value,
        _ => return Err(fail("orientation must be 1..8")),
    };
    let profile: String = match arguments.next() {
        Some(value) => value,
        None => return Err(fail("plain or linear profile required")),
    };
    if arguments.next().is_some() {
        return Err(fail("unexpected argument"));
    }
    let linear: bool = match profile.as_str() {
        "plain" => false,
        "linear" => true,
        _ => return Err(fail("unknown fixture profile")),
    };
    let raster: Raster = read_raster()?;
    verify(&raster, orientation, linear)?;
    println!("JPEG frame: EXIF {orientation}, {profile}, {}x{}, {} bytes, independent color anchors accepted",
        raster.width, raster.height, raster.pixels.len());
    Ok(())
}

fn main() -> std::process::ExitCode {
    let result: Result<(), String> = execute();
    match result {
        Ok(()) => std::process::ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("{error}");
            std::process::ExitCode::FAILURE
        }
    }
}
