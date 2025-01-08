use std::{collections::HashMap, sync::Arc};

use tokio::sync::RwLock;

use crate::http::StatusCode;

#[derive(Debug, Default, Eq, PartialEq, Clone)]
pub struct Stats {
    statuses: HashMap<StatusCode, usize>,
}

pub type StatsPtr = Arc<RwLock<Stats>>;

impl Stats {
    pub fn new() -> Self {
        Stats {
            statuses: HashMap::new(),
        }
    }

    pub fn incr(&mut self, s: StatusCode) {
        todo!("TODO: Part 4");
    }

    pub fn items(&self) -> Vec<(StatusCode, usize)> {
        let mut items = self
            .statuses
            .iter()
            .map(|(&k, &v)| (k, v))
            .collect::<Vec<_>>();
        items.sort_by_key(|&(k, _)| k);
        items
    }
}

pub async fn incr(s: &StatsPtr, sc: StatusCode) {
    todo!("TODO: Part 4");
}
