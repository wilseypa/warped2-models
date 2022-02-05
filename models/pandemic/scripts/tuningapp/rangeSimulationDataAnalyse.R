library(ggplot2)
library(gridExtra)
library(readr)
library(stringr)
library(uuid)
library(svglite) # adding this
library(scales)
library(jsonlite)

# print working directory
getwd()

# needed for ggplot
xf <- c("Confirmed(simulated)", "Active(simulated)", "Deaths(simulated)", 
        "Recovered(simulated)", "Confirmed(actual)", "Active(actual)", 
        "Deaths(actual)", 
        "Recovered(actual)")

xf2 <- c("transmissibility")

plotcolors <- c("Confirmed(simulated)"="palevioletred", 
                "Active(simulated)"="palegreen3", "Deaths(simulated)"="orangered3", 
                "Recovered(simulated)"="royalblue", "Confirmed(actual)"="palevioletred", 
                "Active(actual)"="palegreen3", 
                "Deaths(actual)"="orangered3", 
                "Recovered(actual)"="royalblue")

plotlines <- c("Confirmed(simulated)"="solid", "Active(simulated)"="solid", 
               "Deaths(simulated)"="solid", 
               "Recovered(simulated)"="solid", "Confirmed(actual)"="dashed", 
               "Active(actual)"="dashed", 
               "Deaths(actual)"="dashed", 
               "Recovered(actual)"="dashed")

# get curr dir name
currSimDir <- basename(getwd())

# create new dir under plotImages
dir.create(file.path("../../plotImages", currSimDir), showWarnings = FALSE)

# first copy the metrics file to plots dir (that gets synced to remote)
file.copy(from="./plotSourceData/metrics", to=paste("../../plotImages/", currSimDir, "/", "configParams.txt", sep=''), 
          overwrite = TRUE,
          recursive = FALSE,
          copy.mode = TRUE)

# load list of counties/fips
US_fips_county_name_df <- read.csv("../../US_counties_population_latLong.csv", header=TRUE)
US_fips_county_name_df$FIPS <- as.character(US_fips_county_name_df$FIPS)

dirs_to_plot <- grep('.+', list.dirs("./plotSourceData/", full.names = FALSE), value=TRUE)



# first, create transmissiblity chart

suppressWarnings(rm(tblity_arr))
suppressWarnings(rm(tblitydata))
suppressWarnings(rm(row))
suppressWarnings(rm(newrow))

jsondata <- fromJSON("./plotSourceData/metrics")
tblity_arr <- jsondata$paramsUsed$disease_model$transmissibility

actualUSdata <- read.csv("./plotSourceData/US/actual.csv", header = TRUE)
xf2 <- c("transmissibility")

newrow <- data.frame(Date=as.Date(character()),
                     variable=factor(levels=xf2),
                     value=double())

tblitydata <- data.frame(Date=as.Date(character()),
                         variable=factor(levels=xf2),
                         value=double())

for(i in 1:nrow(actualUSdata)) {
  row <- actualUSdata[i,]
  newrow[1,]$Date <- row$Date
  
  if (length(tblity_arr) == 1) {
    t_expval <- tblity_arr[1] * exp(-1 * i)
    newtval <- t_expval
  } else if (i > length(tblity_arr)) {
    newtval <- tblity_arr[length(tblity_arr)]
  } else {
    newtval <- tblity_arr[i]
  }
  
  newrow[1,]$value <- newtval
  newrow[1,]$variable <- "transmissibility"
  
  tblitydata <- rbind(tblitydata, newrow)
}

pt2 <- ggplot(tblitydata, aes(x=Date,y=value,color=variable)) +
  geom_line(size=0.8) + 
  scale_color_manual(values = c("transmissibility" = "#568c7e")) +
  scale_y_log10() +
  labs(y="transmissibility (log scale)") +
  theme_light() +
  theme(aspect.ratio = 1, legend.position="none", text = element_text(size=9)) +
  scale_x_date(breaks=c(min(newdata$Date), max(newdata$Date)), 
               minor_breaks = list_date_breaks,
               date_labels = "%d %b %Y")


ggsave(paste("../../plotImages/", currSimDir, "/", plotname, ".svg", sep=''), width = 6, height = 6, units = "in", plot=pt1)
ggsave(paste("../../plotImages/", currSimDir, "/", "transmissibility_curve.svg", sep=''), width = 6, height = 6, units = "in", plot=pt2)


# plot disease values in each data directory
for (dir in dirs_to_plot) {
  print(paste("plotting", dir, "..."))
  
  actual_data_filename <- paste("./plotSourceData/", dir, "/actual.csv", sep='')
  simulated_data_filename <- paste("./plotSourceData/", dir, "/simulated.csv", sep='')

  # delete if existing
  suppressWarnings(rm(newdata))
  suppressWarnings(rm(newrow))
  suppressWarnings(rm(actual))
  suppressWarnings(rm(simulated))
  
  # create empty DFs
  newdata <- data.frame(Date=as.Date(character()),
                        Disease_metric=factor(levels=xf),
                        value=integer())        
  newrow <- data.frame(Date=as.Date(character()),
                       Disease_metric=factor(levels=xf),
                       value=integer())
  
  actual <- read.csv(actual_data_filename, header = TRUE)
  actual$Date <- as.Date(actual$Date, format="%m-%d-%Y")

  # add actual data to newdata
  for(i in 1:nrow(actual)) {
    row <- actual[i,]
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Confirmed(actual)"
    newrow[1,]$value <- row$Confirmed
    newdata <- rbind(newdata, newrow)
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Deaths(actual)"
    newrow[1,]$value <- row$Deaths
    newdata <- rbind(newdata, newrow)
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Recovered(actual)"
    newrow[1,]$value <- row$Recovered
    newdata <- rbind(newdata, newrow)
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Active(actual)"
    newrow[1,]$value <- row$Active
    newdata <- rbind(newdata, newrow)
  }
  
  simulated <- read.csv(simulated_data_filename, header=TRUE)
  simulated$Date <- as.Date(simulated$Date, format="%m-%d-%Y")

  # add simulated data to newdata 
  for(i in 1:nrow(simulated)) {
    row <- simulated[i,]
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Confirmed(simulated)"
    newrow[1,]$value <- row$Confirmed
    newdata <- rbind(newdata, newrow)
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Deaths(simulated)"
    newrow[1,]$value <- row$Deaths
    newdata <- rbind(newdata, newrow)
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Recovered(simulated)"
    newrow[1,]$value <- row$Recovered
    newdata <- rbind(newdata, newrow)
    
    newrow[1,]$Date <- row$Date
    newrow[1,]$Disease_metric <- "Active(simulated)"
    newrow[1,]$value <- row$Active
    newdata <- rbind(newdata, newrow)
  }
  
  if (dir != "US") {
    plotname <- as.character(US_fips_county_name_df[US_fips_county_name_df$FIPS == dir,]$County)
  } else {
    plotname <- "US"
  }

  legend.title <- ""
  
  # calc minor breaks for x date axis
  suppressWarnings(rm(list_date_breaks))
  list_date_breaks <- structure(integer(), class = "Date")
  currdate <- min(newdata$Date)
  
  i <- 1
  while (currdate < max(newdata$Date)) {
    # print(currdate)
    list_date_breaks[i] <- currdate
    i <- i + 1
    currdate <- currdate + 30
  }
  
  pt1 <- ggplot(newdata, aes(x=Date, y=value, colour=Disease_metric, 
                linetype=Disease_metric)) +
         geom_line(size=0.6) +
         scale_color_manual(legend.title, values=plotcolors) +
         scale_linetype_manual(legend.title, values=plotlines) +
         labs(y="Count") +
         theme_light() +
         theme(aspect.ratio = 1, legend.position="top", text = element_text(size=9)) +
         scale_y_continuous(name="Count", labels = comma) +
         scale_x_date(breaks=c(min(newdata$Date), max(newdata$Date)), 
                      minor_breaks = list_date_breaks,
                      date_labels = "%d %b %Y")
  
  ggsave(paste("../../plotImages/", currSimDir, "/", plotname, ".pdf", sep=''), width = 6, height = 6, units = "in", plot=pt1)
  ggsave(paste("../../plotImages/", currSimDir, "/", "transmissibility_curve.pdf", sep=''), width = 6, height = 6, units = "in", plot=pt2)
}



