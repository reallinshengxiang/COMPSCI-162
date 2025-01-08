use std::sync::Arc;

use tokio::sync::RwLock;

use crate::stats::*;

#[test]
/// Check that `Stats` has a `new` method.
fn new_method_present() {
    let _ = Stats::new();
}

#[test]
/// Check that `Stats` can be debug printed.
fn stats_debug() {
    let s = Stats::new();
    let _ = format!("can debug print a stats instance: {:?}", s);
}

#[test]
fn incr_items() {
    let counts = [
        (200, 162),
        (304, 12),
        (403, 1),
        (404, 9),
        (501, 5),
        (502, 14),
    ];

    let mut s = Stats::new();

    for (code, count) in counts {
        for _ in 0..count {
            s.incr(code);
        }
    }

    assert_eq!(s.items(), counts);
}

#[test]
fn incr_items_four() {
    let counts = [
        (1, 4 * 123),
        (202, 4 * 32),
        (292, 4 * 3930),
        (303, 4 * 7),
        (343, 4 * 230),
        (293232, 4 * 93),
    ];

    let mut s = Stats::new();

    for _ in 0..4 {
        for (code, count) in counts {
            for _ in 0..(count / 4) {
                s.incr(code);
            }
        }
    }

    assert_eq!(s.items(), counts);
}

#[test]
fn items_sorted() {
    let counts = [
        (393, 162),
        (965, 12),
        (17, 1),
        (483, 9),
        (302, 5),
        (384, 14),
    ];

    let mut sorted = Vec::from(counts);
    sorted.sort_by_key(|v| v.0);

    let mut s = Stats::new();

    for (code, count) in counts {
        for _ in 0..count {
            s.incr(code);
        }
    }

    assert_eq!(s.items(), sorted);
}

#[test]
fn stats_ptr_new() {
    let _: StatsPtr = Arc::new(RwLock::new(Stats::new()));
}

#[tokio::test]
async fn incr_stats_ptr() {
    let counts = [
        (1, 4 * 489),
        (202, 4 * 48),
        (292, 4 * 94),
        (303, 4 * 12),
        (343, 4 * 49),
        (293232, 4 * 65),
    ];

    let s: StatsPtr = Arc::new(RwLock::new(Stats::new()));

    for _ in 0..4 {
        for (code, count) in counts {
            for _ in 0..(count / 4) {
                incr(&s, code).await;
            }
        }
    }

    assert_eq!(s.read().await.items(), counts);
}
