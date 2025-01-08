//! The MapReduce coordinator.
//!

use anyhow::Result;
use tonic::transport::Server;
use tonic::{Request, Response, Status};
use tonic::Code;
use tokio::sync::Mutex;
use tokio::time::Instant;
use std::sync::Arc;
use std::collections::{HashMap, VecDeque};
use tokio::time::Duration;

use crate::log;
use crate::app::named;
use crate::rpc::coordinator::*;
use crate::*;

pub mod args;
const TASK_TIMEOUT_SECS: u64 = 7;

pub struct MapTask{
    pub map_task_number: TaskNumber,
    pub worker_id: WorkerId,
    pub map_task_status: u8,
}

pub struct ReduceTask{
    pub reduce_task_number: TaskNumber,
    pub worker_id: WorkerId,
    pub reduce_task_status: u8,
}

pub struct WorkerTask{
    pub job_id: JobId,
    pub task_number: TaskNumber,
    pub worker_task_status: u8,
}

pub struct Worker {
    pub worker_id: WorkerId,
    pub heartbeat_time: Instant,
    pub worker_status: u8,
    pub job_id: JobId,
    pub job_ids: Vec<JobId>,
    pub task_number: TaskNumber,
    pub worker_task_status: u8,
}

pub struct Job{
    pub files: Vec<String>,
    pub n_reduce: usize,
    pub n_map: usize,
    pub app: String,
    pub output_dir: String,
    pub args: Vec<u8>,
    pub status: u8,
    pub is_full: bool,
    pub map_tasks: Vec<MapTask>,
    pub reduce_tasks: Vec<ReduceTask>,
    pub map_task_assignments: Vec<MapTaskAssignment>,
    pub error_message: Option<String>, 
}

pub struct CoordinatorData {
    pub worker_id: WorkerId,
    pub workers: HashMap<WorkerId, Worker>,
    pub jobs: HashMap<JobId,Job>,
    pub job_id: JobId,
    pub job_queue: VecDeque<JobId>,
}

pub struct Coordinator {
    // TODO: add your own fields
    coordinatordata : Arc<Mutex<CoordinatorData>>,
}

impl Coordinator {
    pub fn new() -> Self {
        let data = CoordinatorData {
            worker_id: 0,
            workers: HashMap::new(),
            jobs: HashMap::new(),
            job_id: 0,
            job_queue: VecDeque::new(),
        };
        Self {coordinatordata: Arc::new(Mutex::new(data)),}
    }
}

#[tonic::async_trait]
impl coordinator_server::Coordinator for Coordinator {
    /// An example RPC.
    ///
    /// Feel free to delete this.
    /// Make sure to also delete the RPC in `proto/coordinator.proto`.
    async fn example(
        &self,
        req: Request<ExampleRequest>,
    ) -> Result<Response<ExampleReply>, Status> {
        let req = req.get_ref();
        let message = format!("Hello, {}!", req.name);
        Ok(Response::new(ExampleReply { message }))
    }

    async fn submit_job(
        &self,
        req: Request<SubmitJobRequest>,
    ) -> Result<Response<SubmitJobReply>, Status> {
        //todo!("Job submission")
        let mut coordinatordata = self.coordinatordata.lock().await;
        let req = req.get_ref();
        coordinatordata.job_id = coordinatordata.job_id + 1;
        let job_id = coordinatordata.job_id; 
        let app_name = named(&req.app);
        match app_name {
            Ok(_) => {},
            Err(e) => return Err(Status::new(Code::InvalidArgument, e.to_string())),
        }
        let mut job = Job {
            files: req.files.clone(),
            n_reduce: req.n_reduce as usize,
            n_map: req.files.len() as usize,
            app: req.app.clone(),
            output_dir: req.output_dir.clone(),
            args: req.args.clone(),
            status: 0,
            is_full: false,
            map_tasks: Vec::new(),
            reduce_tasks: Vec::new(),
            map_task_assignments: Vec::new(),
            error_message: None,
        };
        for task_number in 0..job.files.len() as TaskNumber {
            job.map_tasks.push(MapTask {
                map_task_number: task_number,
                worker_id: 0,
                map_task_status: 0,
            });
        }
        for task_number in 0..job.n_reduce as TaskNumber {
            job.reduce_tasks.push(ReduceTask {
                reduce_task_number: task_number,
                worker_id: 0,
                reduce_task_status: 0,
            });
        }
        coordinatordata.jobs.insert(job_id, job);
        coordinatordata.job_queue.push_back(job_id);
        Ok(Response::new(SubmitJobReply { job_id }))
    }

    async fn poll_job(
        &self,
        req: Request<PollJobRequest>,
    ) -> Result<Response<PollJobReply>, Status> {
        //todo!("Job submission")
        let mut coordinatordata = self.coordinatordata.lock().await;
        let job_id = req.get_ref().job_id;
        let job = match coordinatordata.jobs.get_mut(&job_id) {
            Some(job) => job,
            None => return Err(Status::new(Code::NotFound, "Invalid")),
        };
        /*Ok(Response::new(PollJobReply { 
            done: job.status == 3,
            failed: false,
            errors: Vec::new(),
        }))*/
        match job.status {
            3 => Ok(Response::new(PollJobReply { 
                done: true,
                failed: false,
                errors: Vec::new(),
            })),
            4 => Ok(Response::new(PollJobReply { 
                    done: true,
                    failed: true,
                    //errors: job.error_message.clone(),
                    errors: match &job.error_message {
                        Some(err) => vec![err.clone()],
                        None => Vec::new(),
                    },
            })),
            _ => Ok(Response::new(PollJobReply { 
                done: false,
                failed: false,
                errors: Vec::new(),
            })),
        }        
    }

    async fn heartbeat(
        &self,
        req: Request<HeartbeatRequest>,
    ) -> Result<Response<HeartbeatReply>, Status> {
        // TODO: Worker registration
        //Ok(Response::new(HeartbeatReply {}))
        let mut coordinatordata = self.coordinatordata.lock().await;
        let worker_id = req.get_ref().worker_id;
        match coordinatordata.workers.get_mut(&worker_id) {
            Some(worker) => {
                worker.heartbeat_time = Instant::now();
                log::info!("Heartbeat from worker {}", worker.worker_id);
                Ok(Response::new(HeartbeatReply {}))
            }
            None => Err(Status::not_found("Worker not found")),
        }
    }

    async fn register(
        &self,
        _req: Request<RegisterRequest>,
    ) -> Result<Response<RegisterReply>, Status> {
        // TODO: Worker registration
        //Ok(Response::new(RegisterReply { worker_id: 0 }))
        let mut coordinatordata = self.coordinatordata.lock().await;
        coordinatordata.worker_id = coordinatordata.worker_id + 1;
        let worker_id = coordinatordata.worker_id;
        let last_heartbeat_time = Instant::now();
        coordinatordata.workers.insert(worker_id, Worker {
            worker_id: worker_id,
            heartbeat_time: last_heartbeat_time,
            worker_status: 0,
            job_id: 0,
            job_ids: Vec::new(),
            task_number: 0,
            worker_task_status: 0,
        });
        log::info!("Registered worker {}", worker_id);
        Ok(Response::new(RegisterReply { worker_id:worker_id }))
    }

    async fn get_task(
        &self,
        req: Request<GetTaskRequest>,
    ) -> Result<Response<GetTaskReply>, Status> {
        // TODO: Tasks
        /*Ok(Response::new(GetTaskReply {
            job_id: 0,
            output_dir: "".to_string(),
            app: "".to_string(),
            task: 0,
            file: "".to_string(),
            n_reduce: 0,
            n_map: 0,
            reduce: false,
            wait: true,
            map_task_assignments: Vec::new(),
            args: Vec::new(),
        }))*/
        let mut task = GetTaskReply {
            job_id: 0,
            output_dir: "".to_string(),
            app: "".to_string(),
            task: 0,
            file: "".to_string(),
            n_reduce: 0,
            n_map: 0,
            reduce: false,
            wait: true,
            map_task_assignments: Vec::new(),
            args: Vec::new(),
        };
        let mut coordinatordata = self.coordinatordata.lock().await;
        let job_queue = coordinatordata.job_queue.clone();
        let worker_id = req.get_ref().worker_id;
        let current_time = Instant::now();
        let mut timed_out_workers = vec![];
        {
            let mut workers_to_remove = Vec::new();
            for (&id, worker) in coordinatordata.workers.iter_mut() {
                if current_time.duration_since(worker.heartbeat_time) > Duration::from_secs(TASK_TIMEOUT_SECS) {
                    log::warn!("Worker {} has timed out", id);
                    worker.worker_status = 2;
                    for worker_job_id in &worker.job_ids {
                        timed_out_workers.push((id, *worker_job_id));
                        println!("Job ID: {}", *worker_job_id);
                    }
                    workers_to_remove.push(id); 
                }
            }
            for id in workers_to_remove {
                coordinatordata.workers.remove(&id);
            }
        }
        for (worker_id, job_id) in timed_out_workers {
            if let Some(job) = coordinatordata.jobs.get_mut(&job_id) {
                for map_task in &mut job.map_tasks {
                    if map_task.worker_id == worker_id {
                        job.status = 1;
                        map_task.worker_id = 0;
                        map_task.map_task_status = 0;
                        log::info!("Reassigning map task {}", map_task.map_task_number);
                    }
                }
                for reduce_task in &mut job.reduce_tasks {
                    if reduce_task.worker_id == worker_id  {
                        reduce_task.worker_id = 0;
                        reduce_task.reduce_task_status = 0;
                        log::info!("Reassigning reduce task {}", reduce_task.reduce_task_number);
                    }
                }
                job.map_task_assignments.retain(|assignment| assignment.worker_id != worker_id);
                job.is_full = false;
                if job.map_tasks.iter().all(|task| task.map_task_status == 1) || job.reduce_tasks.iter().all(|task| task.reduce_task_status == 1) {
                    job.is_full = true;
                }
                if !coordinatordata.job_queue.contains(&job_id) {
                    coordinatordata.job_queue.push_back(job_id);
                }
            }
        }
        let job_id = match job_queue.iter().find(|job_id| {
            matches!(coordinatordata.jobs.get_mut(job_id).unwrap().is_full, false)
        }){
            Some(&job_id) => job_id,
            None => return Ok(Response::new(task)),
        };
        let job = coordinatordata.jobs.get_mut(&job_id).unwrap();
        task.job_id = job_id;
        task.app = job.app.clone();
        task.output_dir = job.output_dir.clone();
        task.args = job.args.clone();
        task.n_reduce = job.n_reduce as u32;
        task.n_map = job.files.len() as u32;
        task.map_task_assignments = job.map_task_assignments.clone();
        if job.status == 0 {
            job.status = 1;
        }
        log::info!("get_task  job_id:{} job_status:{} Task assignments: {:?}", job_id, job.status, task.map_task_assignments);
        match job.status {
            1 => {
                let mut map_task = None;
                for task in &mut job.map_tasks {
                    if matches!(task.map_task_status, 0) {
                        map_task = Some(task);
                        break;
                    }
                }
                if map_task.is_none() {
                    let task = GetTaskReply {
                        job_id: 0,
                        output_dir: "".to_string(),
                        app: "".to_string(),
                        task: 0,
                        file: "".to_string(),
                        n_reduce: 0,
                        n_map: 0,
                        reduce: false,
                        wait: true,
                        map_task_assignments: Vec::new(),
                        args: Vec::new(),
                    };
                    return Ok(Response::new(task));
                } 
                let map_task = map_task.unwrap();
                map_task.worker_id = worker_id;
                map_task.map_task_status = 1;
                task.task = map_task.map_task_number as u32;
                task.file = job.files[map_task.map_task_number as usize].clone();
                task.reduce = false;
                task.wait = false;
                if map_task.map_task_number == job.files.len() - 1 {
                    job.is_full = true;
                }
            },
            2 => {
                let mut reduce_task = None; 
                for task in &mut job.reduce_tasks {
                    if matches!(task.reduce_task_status, 0) {
                        reduce_task = Some(task);
                        break;
                    }
                }
                if reduce_task.is_none() {
                    let task = GetTaskReply {
                        job_id: 0,
                        output_dir: "".to_string(),
                        app: "".to_string(),
                        task: 0,
                        file: "".to_string(),
                        n_reduce: 0,
                        n_map: 0,
                        reduce: false,
                        wait: true,
                        map_task_assignments: Vec::new(),
                        args: Vec::new(),
                    };
                    return Ok(Response::new(task));
                } 
                let reduce_task = reduce_task.unwrap();
                reduce_task.reduce_task_status = 1; 
                reduce_task.worker_id = worker_id;
                task.task = reduce_task.reduce_task_number as u32;
                task.reduce = true;
                task.wait = false;
                if reduce_task.reduce_task_number == job.n_reduce - 1 {
                    job.is_full = true;
                }
            }, 
            _ => return Ok(Response::new(task)),
        }
        let worker = coordinatordata.workers.get_mut(&worker_id).unwrap();
        worker.worker_status = 1;
        worker.job_id = job_id;
        if !worker.job_ids.contains(&job_id) {
            worker.job_ids.push(job_id);
        }
        worker.task_number = task.task as usize;
        worker.worker_task_status = task.reduce as u8;  
        log::info!("get_task  worker_id:{} worker_job_id:{} worker_task_number:{} worker_task_status:{}", worker.worker_id, worker.job_id, worker.task_number, worker.worker_task_status);
        Ok(Response::new(task))
    }
    
    async fn finish_task(
        &self,
        req: Request<FinishTaskRequest>,
    ) -> Result<Response<FinishTaskReply>, Status> {
        let mut coordinatordata = self.coordinatordata.lock().await;
        let req = req.into_inner();
        let worker_id = req.worker_id;
        let worker = coordinatordata.workers.get_mut(&worker_id).unwrap();
        worker.worker_status = 0;
        let job_id = req.job_id;
        let job = coordinatordata.jobs.get_mut(&job_id).unwrap();
        if req.reduce {
            let mut reduce_task = None;
            for task in &mut job.reduce_tasks {
                if task.reduce_task_number == req.task as usize {
                    reduce_task = Some(task);
                    break;
                }
            }
            let reduce_task = reduce_task.unwrap();
            log::info!("finish_task   reduce_task_id  : {} reduce_task_status: {}", reduce_task.reduce_task_number, reduce_task.reduce_task_status);
            reduce_task.reduce_task_status = 2;
            let mut all_completed = true;
            for reduce_task in &job.reduce_tasks {
                if reduce_task.reduce_task_status != 2 {
                    all_completed = false;
                    break;
                }
            }
            if all_completed {
                log::info!("finish_task:reduce_all_completed!");
                job.status = 3;
                coordinatordata.job_queue.retain(|&jobid| jobid != job_id);
                for (_, pos_worker) in coordinatordata.workers.iter_mut() {
                    pos_worker.job_ids.retain(|&jobid| jobid != job_id);
                }
                log::info!("finish_task:reduce_all_completed! job_queue {:?}", coordinatordata.job_queue);
            }
        } else {
            let mut map_task = None;
            for task in &mut job.map_tasks {
                if task.map_task_number == req.task as usize {
                    map_task = Some(task);
                    break;
                }
            }
            let map_task = map_task.unwrap();
            log::info!("finish_task   map_task   map_task_id  : {} map_task_status: {}", map_task.map_task_number, map_task.map_task_status);
            map_task.map_task_status = 2;
            let mut all_completed = true;
            for map_task in &job.map_tasks {
                if map_task.map_task_status != 2 {
                    all_completed = false;
                    break;
                }
            }
            if all_completed {
                log::info!("finish_task:map_all_completed!");
                job.status = 2;
                job.is_full = false;
                job.map_task_assignments = Vec::new();
                for i in 0..job.map_tasks.len() {
                    let task = job.map_tasks[i].map_task_number as u32;
                    let worker_id = job.map_tasks[i].worker_id;
                    if worker_id != 0 {
                        log::info!("task:{} worker_id:{} ",task, worker_id );
                        let map_task_assignment = Response::new(MapTaskAssignment { task, worker_id }); 
                        job.map_task_assignments.push(map_task_assignment.into_inner());
                    }
                }
            }
        }
         Ok(Response::new(FinishTaskReply {}))
    }

    async fn fail_task(
        &self,
        req: Request<FailTaskRequest>,
    ) -> Result<Response<FailTaskReply>, Status> {
        // TODO: Fault tolerance
       // Ok(Response::new(FailTaskReply {}))
       let mut coordinatordata = self.coordinatordata.lock().await;
       let req = req.into_inner();
       let worker_id = req.worker_id;
       let job_id = req.job_id;
       let task_number = req.task;
       let is_reduce = req.reduce;
       let should_retry = req.retry;
       let error_message = req.error.clone();
       let job = match coordinatordata.jobs.get_mut(&job_id) {
           Some(job) => job,
           None => return Err(Status::not_found("Job ID not found")),
       };
       job.error_message = Some(error_message.clone());
       eprintln!(
           "Task failed: worker_id={}, job_id={}, task={}, reduce={}, retry={}, error={}",
           worker_id, job_id, task_number, is_reduce, should_retry, error_message
       );
       if is_reduce { 
           let all_maps_completed = job.map_tasks.iter().all(|task| task.map_task_status == 2);
            if !all_maps_completed {
                log::warn!(
                    "Cannot process reduce task {} for job {} because not all map tasks are completed.",
                    task_number,
                    job_id
                );
                return Err(Status::failed_precondition("Not all map tasks are completed"));
            }
       }   
       if should_retry {
            if is_reduce {
                if let Some(reduce_task) = job.reduce_tasks.iter_mut().find(|t| t.reduce_task_number == task_number as usize) {
                    reduce_task.reduce_task_status = 0; 
                    reduce_task.worker_id = 0;        
                }
                else {
                    job.reduce_tasks.push(ReduceTask {
                        reduce_task_number: task_number as usize,
                        worker_id: 0, 
                        reduce_task_status: 0,
                    });
                }
            } else {
                if let Some(map_task) = job.map_tasks.iter_mut().find(|t| t.map_task_number == task_number as usize) {
                    map_task.map_task_status = 0; 
                    map_task.worker_id = 0;  
                }
                else {
                    job.map_tasks.push(MapTask {
                        map_task_number: task_number as usize, 
                        worker_id: 0,
                        map_task_status: 0, 
                    });
                }
            }
       }
       else {
           log::error!("Job {} failed due to task failure", job_id);
           job.status = 4; 
           job.is_full = true;
           for task in job.map_tasks.iter_mut() {
               task.map_task_status = 3;
           }
           for task in job.reduce_tasks.iter_mut() {
               task.reduce_task_status = 3;
           }
           //return Ok(Response::new(FailTaskReply {})); 
           return Err(Status::failed_precondition("stop!"));
       }
       if let Some(worker) = coordinatordata.workers.get_mut(&worker_id) {
           worker.worker_status = 0; 
           worker.job_id = 0;
           worker.task_number = 0;
       }
       Ok(Response::new(FailTaskReply {}))
    }
}

pub async fn start(_args: args::Args) -> Result<()> {
    let addr = COORDINATOR_ADDR.parse().unwrap();

    let coordinator = Coordinator::new();
    let svc = coordinator_server::CoordinatorServer::new(coordinator);
    Server::builder().add_service(svc).serve(addr).await?;

    Ok(())
}