library(ggplot2)
library(gridExtra)
library(readr)
library(stringr)
library(uuid)
library(svglite) # adding this
library(scales)

# print working directory
getwd()

# get curr dir name
currSimDir <- basename(getwd())

# create new dir under plotImages
dir.create(file.path("../../plotImages", currSimDir), showWarnings = FALSE)

# load list of counties/fips
US_fips_county_name_df <- read.csv("../../US_counties_population_latLong.csv", header=TRUE)
US_fips_county_name_df$FIPS <- as.character(US_fips_county_name_df$FIPS)

dirs_to_plot <- grep('.+', list.dirs("./plotSourceData/", full.names = FALSE), value=TRUE)


# # get input params used for simulation
# params_string <- read_file(paste("./plotSourceData/", "metrics", sep=''))
# # plot_annotate_metrics <- ggplot() + annotate("text", x = 1, y = 1, label = stringr::str_wrap(params_string), size=8)
# label_string <- paste(params_string, "", "Colors denote different disease metrics:", "palered : Confirmed", "orange : Deaths",
#                       "blue : Recovered", "green : Active", "", 
#                       "Solid line represents Simulation data", 
#                       "Dashed line represents Actual data", sep='\n')
# plot_annotate_metrics <- ggplot() + xlim(1,3) + annotate("text",x=1,y=1, label = label_string, size=6, hjust=0)
# 
# plots <- list(plot_annotate_metrics)

# i <- 2
for (dir in dirs_to_plot) {
  print(paste("plotting", dir, "..."))
  
  actual_data_filename <- paste("./plotSourceData/", dir, "/actual.csv", sep='')
  simulated_data_filename <- paste("./plotSourceData/", dir, "/simulated.csv", sep='')

  # delete if existing
  rm(newdata)
  rm(newrow)
  rm(actual)
  rm(simulated)
  
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
    # print(row)
    # do stuff with row
    
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
    # print(row)
    # do stuff with row
    
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
  rm(list_date_breaks)
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
                      minor_breaks = list_date_breaks, date_labels = "%d %b %Y")

  ggsave(paste("../../plotImages/", currSimDir, "/", plotname, ".pdf", sep=''), width = 6, height = 6, units = "in", plot=pt1)
  
  
  
  # plot <- ggplot() +
  #   geom_line(data = actual, aes(x=Date, y=Confirmed), color='palevioletred', linetype="dashed", lwd=2.0) +
  #   geom_line(data = actual, aes(x=Date, y=Deaths), color='orangered3', linetype="dashed", lwd=2.0) +
  #   geom_line(data = actual, aes(x=Date, y=Recovered), color='royalblue', linetype="dashed", lwd=2.0) +
  #   geom_line(data = actual, aes(x=Date, y=Active), color='palegreen3', linetype="dashed", lwd=2.0) +
  #   geom_line(data = simulated, aes(x=Date, y=Confirmed), color='palevioletred', lwd=2.0) +
  #   geom_line(data = simulated, aes(x=Date, y=Deaths), color='orangered3', lwd=2.0) +
  #   geom_line(data = simulated, aes(x=Date, y=Recovered), color='royalblue', lwd=2.0) +
  #   geom_line(data = simulated, aes(x=Date, y=Active), color='palegreen3', lwd=2.0) +
  #   labs(title = plotname)
  
  
  #   # labs(title = "'Confirmed' cases count for Los Angeles (simulated vs. actual)",
  #   #      caption = "Blue: simulated, Gray: actual")
  # 
  # # class(plot)
  # plots[[i]] <- plot
  # i <- i + 1
}



# 
# no_of_plots <- length(plots)
# 
# print(no_of_plots)
# 
# base_square_side <- floor(sqrt(no_of_plots))
# 
# if (base_square_side == sqrt(no_of_plots)) {
#   png_dim_x <- base_square_side * 800
#   png_dim_y <- base_square_side * 800
# } else if (no_of_plots - (base_square_side ^ 2) <= base_square_side) {
#   png_dim_x <- (base_square_side + 1) * 800
#   png_dim_y <- base_square_side * 800
# } else {
#   png_dim_x <- (base_square_side + 1) * 800
#   png_dim_y <- (base_square_side + 1) * 800
# }
# 
# # png_dim <- ceiling(sqrt(no_of_plots)) * 800
# 
# png(paste("../../plotImages/plot_", UUIDgenerate(), ".png", sep=''), width=png_dim_x, height=png_dim_y)
# # plot(1)
# # title(sub="hallo", adj=1, line=3, font=2)
# do.call("grid.arrange", c(plots, ncol=ceiling(sqrt(no_of_plots))))
# 
# # grid.arrange(plot_annotate, plots, ncol=2)
# dev.off()




